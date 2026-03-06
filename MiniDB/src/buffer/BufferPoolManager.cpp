//
//  BufferPoolManager.cpp
//  MiniDB
//
//  Created by 张宗煜 on 2026/1/5.
//

#include "BufferPoolManager.h"
#include "Page.h"
#include "LayeredClock.h"
#include "DiskManager.h"
#include "config.h"
#include <mutex>
#include <vector>
#include <unordered_map>
#include <cstddef>
#include <iostream>

BufferPoolManager::BufferPoolManager(size_t pool_size, DiskManager* disk_manager){
    this->pool_size = pool_size;
    this->disk_manager = disk_manager;
    this->replacer = new LayeredClock(pool_size);
    this->pages.resize(pool_size, nullptr);
}

BufferPoolManager::~BufferPoolManager(){
    for (size_t i = 0; i < pages.size(); ++i) {
        if (pages[i] != nullptr) {
            delete pages[i];
        }
    }
    if (replacer != nullptr) {
        delete replacer;
    }
}

Page* BufferPoolManager::FetchPage(int page_id){
    std::lock_guard<std::mutex> lock(latch);
    
    auto it = page_table.find(page_id);
    if (it != page_table.end()) {
        size_t frame_id = it->second;
        Page* page = pages[frame_id];
        page->Pin();
        replacer->Pin(static_cast<int>(frame_id));
        return page;
    }
    size_t frame_id = 0;
    bool found_free = false;
    
    for (size_t i = 0; i < pool_size; ++i) {
        if (pages[i] == nullptr) {
            frame_id = i;
            found_free = true;
            break;
        }
    }
    if (!found_free) {
        int victim_frame_id = -1;
        if (!replacer->Victim(victim_frame_id)) {
            return nullptr;
        }
        frame_id = static_cast<size_t>(victim_frame_id);
        
        Page* victim_page = pages[frame_id];
        if (victim_page != nullptr) {
            if (victim_page->GetPinCount() > 0) {
                return nullptr;
            }
            if (victim_page->IsDirty()) {
                disk_manager->WritePage(victim_page->GetPageId(), 
                                       reinterpret_cast<const char*>(victim_page->GetData()), 
                                       PAGE_SIZE);
            }
            page_table.erase(victim_page->GetPageId());
        }
    }
    
    if (pages[frame_id] == nullptr) {
        pages[frame_id] = new Page();
    }
    Page* page = pages[frame_id];
    page->Reset();
    page->SetPageId(page_id);
    char* page_data = const_cast<char*>(reinterpret_cast<const char*>(page->GetData()));
    disk_manager->ReadPage(page_id, page_data);
    page_table[page_id] = frame_id;
    page->Pin();
    replacer->Pin(static_cast<int>(frame_id));
    return page;
}

bool BufferPoolManager::UnpinPage(int page_id, bool is_dirty){
    std::lock_guard<std::mutex> lock(latch);
    auto it = page_table.find(page_id);
    if (it == page_table.end()) {
        return false;
    }
    size_t frame_id = it->second;
    Page* page = pages[frame_id];
    if (page->GetPinCount() == 0) {
        return false;
    }
    page->Unpin();
    if (is_dirty) {
        page->SetDirty(true);
    }
    if (page->GetPinCount() == 0) {
        bool is_hot = (page->GetTemperature() == PageTemperature::HOT);
        replacer->Unpin(static_cast<int>(frame_id), is_hot);
    }
    return true;
}

bool BufferPoolManager::FlushPage(int page_id) {
    std::lock_guard<std::mutex> lock(latch);
    return FlushPageInternal(page_id);
}

bool BufferPoolManager::FlushPageInternal(int page_id) {
    auto it = page_table.find(page_id);
    if (it == page_table.end()) return false;
    size_t frame_id = it->second;
    Page* page = pages[frame_id];
    if (page->IsDirty()) {
        disk_manager->WritePage(page_id,
            reinterpret_cast<const char*>(page->GetData()), PAGE_SIZE);
        page->SetDirty(false);
    }
    return true;
}

Page* BufferPoolManager::NewPage(int *page_id) {
    std::lock_guard<std::mutex> lock(latch);
    size_t frame_id = 0;
    bool found_free = false;
    for (size_t i = 0; i < pool_size; ++i) {
        if (pages[i] == nullptr) {
            frame_id = i;
            found_free = true;
            break;
        }
    }
    if (!found_free) {
        int victim_frame_id = -1;
        if (!replacer->Victim(victim_frame_id)) {
            return nullptr;
        }
        frame_id = static_cast<size_t>(victim_frame_id);
        
        Page* victim_page = pages[frame_id];
        if (victim_page != nullptr) {
            if (victim_page->GetPinCount() > 0) {
                return nullptr;
            }
            if (victim_page->IsDirty()) {
                disk_manager->WritePage(victim_page->GetPageId(), 
                                       reinterpret_cast<const char*>(victim_page->GetData()), 
                                       PAGE_SIZE);
            }
            page_table.erase(victim_page->GetPageId());
        }
    }
    *page_id = disk_manager->AllocPage();

    if (pages[frame_id] == nullptr) {
        pages[frame_id] = new Page();
    }
    Page* page = pages[frame_id];
    page->Reset();
    page->SetPageId(*page_id);
    page->Pin();
    page->SetDirty(false);
    page_table[*page_id] = frame_id;
    replacer->Pin(static_cast<int>(frame_id));
    return page;
}
bool BufferPoolManager::DeletePage(int page_id) {
    std::lock_guard<std::mutex> lock(latch);
    auto it = page_table.find(page_id);
    if (it == page_table.end()) {
        return false;
    }
    size_t frame_id = it->second;
    Page* page = pages[frame_id];
    if (page->GetPinCount() > 0) {
        return false;
    }
    if (page->IsDirty()) {
        disk_manager->WritePage(page_id, 
                               reinterpret_cast<const char*>(page->GetData()), 
                               PAGE_SIZE);
    }
    
    page_table.erase(page_id);
    page->Reset();
    pages[frame_id] = nullptr;
    replacer->Pin(static_cast<int>(frame_id));  // 从替换器中移除（与 README 一致）
    return true;
}

void BufferPoolManager::FlushAllPages() {
    std::lock_guard<std::mutex> lock(latch);
    for (auto& pair : page_table)
        FlushPageInternal(pair.first);
}

bool BufferPoolManager::isHotPage(int page_id) {
    auto it = page_table.find(page_id);
    if (it == page_table.end()) {
        return false;
    }
    size_t frame_id = it->second;
    Page* page = pages[frame_id];
    if (page == nullptr) {
        return false;
    }
    return page->GetTemperature() == PageTemperature::HOT;
}

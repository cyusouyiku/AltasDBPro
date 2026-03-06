//
//  BufferPoolManager.h
//  MiniDB
//
//  Created by 张宗煜 on 2026/1/4.
//

#ifndef BUFFERPOOLMANAGER_HPP
#define BUFFERPOOLMANAGER_HPP

#include "Page.h"
#include "LayeredClock.h"

class DiskManager;
#include <mutex>
#include <vector>
#include <unordered_map>
#include <cstddef>

class BufferPoolManager {
private:
    std::vector<Page*> pages;
    std::unordered_map<int, size_t> page_table;
    LayeredClock* replacer;
    DiskManager* disk_manager;
    size_t pool_size;
    std::mutex latch;
 
public:
    BufferPoolManager(size_t pool_size, DiskManager* disk_manager);
    ~BufferPoolManager();
    Page* FetchPage(int page_id);
    bool UnpinPage(int page_id, bool is_dirty);
    bool FlushPage(int page_id);
    Page* NewPage(int* page_id);
    bool DeletePage(int page_id);
    void FlushAllPages();
    bool isHotPage(int page_id);

private:
    bool FlushPageInternal(int page_id);
};

#endif

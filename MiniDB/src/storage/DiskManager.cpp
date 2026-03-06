//
//  DiskManager.cpp
//  MiniDB
//
//  Created by 张宗煜 on 2026/1/3.
//

#include "DiskManager.h"
#include "Page.h"
#include <string>
#include <mutex>
#include <fstream>
#include "config.h"

DiskManager::DiskManager(const std::string& db_file_path)
    : db_file_path(db_file_path), page_id(0), next_page_id(0), file_size(0) {
    OpenManager();
}

DiskManager::DiskManager(std::string db_file_path, std::fstream db_file_fd, int page_id, int next_page_id, int file_size){
    this->db_file_path = db_file_path;
    this->db_file_fd = std::move(db_file_fd);
    this->page_id = page_id;
    this->next_page_id = next_page_id;
    this->file_size = file_size;
}

DiskManager::~DiskManager(){};

void DiskManager::OpenManager() {
    db_file_fd.open(db_file_path,
                    std::ios::in | std::ios::out | std::ios::binary);

    if (!db_file_fd.is_open()) {
        db_file_fd.open(db_file_path,
                        std::ios::out | std::ios::binary);
        db_file_fd.close();

        db_file_fd.open(db_file_path,
                        std::ios::in | std::ios::out | std::ios::binary);
    }

    db_file_fd.seekg(0, std::ios::end);
    file_size = static_cast<int>(db_file_fd.tellg());
    next_page_id = file_size / PAGE_SIZE;
}

void DiskManager::ReadPage(int page_id, char *page_data) {
    std::lock_guard<std::mutex> lock(latch);
    int offset = page_id * PAGE_SIZE;
    db_file_fd.seekg(offset, std::ios::beg);
    db_file_fd.read(page_data, PAGE_SIZE);
}

void DiskManager::WritePage(int page_id, const char* external_buffer, size_t buffer_size) {
    std::lock_guard<std::mutex> lock(latch);
    db_file_fd.open(db_file_path, std::ios::in | std::ios::out | std::ios::binary);
    size_t offset = (size_t)page_id * PAGE_SIZE;
    db_file_fd.seekp(offset, std::ios::beg);
    size_t write_size = (buffer_size < PAGE_SIZE) ? buffer_size : PAGE_SIZE;
    db_file_fd.write(external_buffer, write_size);
    db_file_fd.close();
}

int DiskManager::AllocPage() {
    std::lock_guard<std::mutex> lock(latch);
    int new_page_id = next_page_id;
    next_page_id++;
    file_size += PAGE_SIZE;
    return new_page_id;
}

void DiskManager::FlushPage() {
}

void DiskManager::FlushAllPages() {
    std::lock_guard<std::mutex> lock(latch);
    if (db_file_fd.is_open()) {
        db_file_fd.flush();
    }
}
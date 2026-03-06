//
//  buffer_pool_test.cpp
//  MiniDB
//
//  Created by Auto-generated on 2026/1/24.
//

#include "../MiniDB/include/buffer/BufferPoolManager.h"
#include "../MiniDB/include/storage/DiskManager.h"
#include <iostream>
#include <cassert>

// 缓冲池测试
void TestBufferPoolManager() {
    std::cout << "Testing BufferPoolManager..." << std::endl;

    DiskManager* disk_manager = new DiskManager("test.db");
    BufferPoolManager* bpm = new BufferPoolManager(10, disk_manager);

    // 测试基本功能
    int page_id = 0;
    Page* page = bpm->NewPage(&page_id);
    assert(page != nullptr);
    std::cout << "NewPage test passed" << std::endl;

    bpm->UnpinPage(page_id, true);
    bpm->FlushPage(page_id);

    delete bpm;
    delete disk_manager;

    std::cout << "BufferPoolManager tests completed" << std::endl;
}

int main() {
    TestBufferPoolManager();
    return 0;
}
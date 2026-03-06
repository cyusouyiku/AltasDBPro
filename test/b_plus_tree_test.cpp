//
//  b_plus_tree_test.cpp
//  MiniDB
//
//  Created by Auto-generated on 2026/1/24.
//

#include "../MiniDB/include/index/BPlusTree.h"
#include "../MiniDB/include/buffer/BufferPoolManager.h"
#include "../MiniDB/include/storage/DiskManager.h"
#include <iostream>
#include <cassert>

// B+ 树测试
void TestBPlusTree() {
    std::cout << "Testing BPlusTree..." << std::endl;

    DiskManager* disk_manager = new DiskManager("test.db");
    BufferPoolManager* bpm = new BufferPoolManager(10, disk_manager);

    BPlusTree* bpt = new BPlusTree("test_index", bpm, 50, 25);

    RID rid(0, 0);
    bool success = bpt->Insert(1, rid);
    assert(success);
    std::cout << "Insert test passed" << std::endl;

    std::vector<RID> result;
    success = bpt->GetValue(1, &result);
    assert(success && !result.empty());
    std::cout << "GetValue test passed" << std::endl;

    delete bpt;
    delete bpm;
    delete disk_manager;

    std::cout << "BPlusTree tests completed" << std::endl;
}

int main() {
    TestBPlusTree();
    return 0;
}
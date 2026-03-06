//
//  executor_test.cpp
//  MiniDB
//
//  Created by Auto-generated on 2026/1/24.
//

#include "../MiniDB/include/executor/SeqScanExecutor.h"
#include "../MiniDB/include/storage/table_heap.h"
#include "../MiniDB/include/buffer/BufferPoolManager.h"
#include "../MiniDB/include/storage/DiskManager.h"
#include <iostream>
#include <cassert>

// 执行器测试
void TestExecutor() {
    std::cout << "Testing Executor..." << std::endl;

    DiskManager* disk_manager = new DiskManager("test.db");
    BufferPoolManager* bpm = new BufferPoolManager(10, disk_manager);
    TableHeap* table = new TableHeap(bpm);

    SeqScanExecutor* executor = new SeqScanExecutor(table);
    executor->Init();

    Tuple tuple;
    RID rid;
    bool has_next = executor->Next(&tuple, &rid);
    std::cout << "SeqScanExecutor test: " << (has_next ? "has data" : "no data") << std::endl;

    delete executor;
    delete table;
    delete bpm;
    delete disk_manager;

    std::cout << "Executor tests completed" << std::endl;
}

int main() {
    TestExecutor();
    return 0;
}
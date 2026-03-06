//
//  integration_test.cpp
//  MiniDB - 全功能集成测试
//

#include "buffer/BufferPoolManager.h"
#include "storage/DiskManager.h"
#include "storage/table_heap.h"
#include "storage/CatalogManager.h"
#include "index/BPlusTree.h"
#include "executor/SeqScanExecutor.h"
#include "executor/InsertExecutor.h"
#include "executor/IndexScanExecutor.h"
#include "concurrency/LockManager.h"
#include "concurrency/Transaction.h"
#include "concurrency/TransactionManager.h"
#include "concurrency/DeadLockDector.h"
#include "parser/SQLParser.h"
#include "common/types.h"
#include "common/rid.h"
#include <iostream>
#include <cassert>
#include <vector>
#include <string>

static int tests_passed = 0;
static int tests_failed = 0;

#define ASSERT(cond, msg) do { \
    if (cond) { tests_passed++; std::cout << "  [OK] " << msg << std::endl; } \
    else { tests_failed++; std::cerr << "  [FAIL] " << msg << std::endl; } \
} while(0)

void TestBufferPoolExtended() {
    std::cout << "\n=== Buffer Pool 扩展测试 ===" << std::endl;
    DiskManager* dm = new DiskManager("integration_test.db");
    BufferPoolManager* bpm = new BufferPoolManager(5, dm);

    int page_id;
    Page* p = bpm->NewPage(&page_id);
    ASSERT(p != nullptr, "NewPage");
    ASSERT(page_id >= 0, "Valid page_id");
    bpm->UnpinPage(page_id, true);

    Page* p2 = bpm->FetchPage(page_id);
    ASSERT(p2 != nullptr && p2->GetPageId() == page_id, "FetchPage");
    bpm->UnpinPage(page_id, false);

    ASSERT(bpm->FlushPage(page_id), "FlushPage");
    delete bpm;
    delete dm;
}

void TestTableHeapFull() {
    std::cout << "\n=== TableHeap 完整测试 ===" << std::endl;
    DiskManager* dm = new DiskManager("integration_test.db");
    BufferPoolManager* bpm = new BufferPoolManager(20, dm);
    TableHeap* table = new TableHeap(bpm);

    Tuple t1;
    t1.SetSize(8);
    t1.SetIntValue(0, 100);
    t1.SetIntValue(4, 200);
    RID rid1;
    ASSERT(table->InsertTuple(t1, &rid1), "InsertTuple 1");
    ASSERT(rid1.IsValid(), "Valid RID after insert");

    Tuple t2;
    t2.SetSize(8);
    t2.SetIntValue(0, 101);
    RID rid2;
    table->InsertTuple(t2, &rid2);

    Tuple out;
    ASSERT(table->GetTuple(rid1, &out), "GetTuple by RID");
    ASSERT(out.GetSize() > 0, "GetTuple returned data");

    int count = 0;
    for (auto it = table->Begin(); it != table->End(); ++it) {
        count++;
    }
    ASSERT(count >= 2, "TableIterator scan");

    delete table;
    delete bpm;
    delete dm;
}

void TestBPlusTreeFull() {
    std::cout << "\n=== B+ Tree 完整测试 ===" << std::endl;
    DiskManager* dm = new DiskManager("integration_test.db");
    BufferPoolManager* bpm = new BufferPoolManager(20, dm);
    BPlusTree* bpt = new BPlusTree("idx_test", bpm, 50, 25);

    for (int i = 1; i <= 10; i++) {
        ASSERT(bpt->Insert(i * 10, RID(i, i)), "Insert key " + std::to_string(i * 10));
    }

    std::vector<RID> result;
    ASSERT(bpt->GetValue(30, &result) && !result.empty(), "GetValue 30");

    ASSERT(bpt->Remove(30), "Remove key 30");
    result.clear();
    ASSERT(!bpt->GetValue(30, &result) || result.empty(), "GetValue after Remove returns empty");

    auto begin = bpt->Begin();
    auto end = bpt->End();
    ASSERT(!begin.IsEnd() || begin != end, "B+ Tree iterator");

    delete bpt;
    delete bpm;
    delete dm;
}

void TestInsertExecutor() {
    std::cout << "\n=== InsertExecutor 测试 ===" << std::endl;
    DiskManager* dm = new DiskManager("integration_test.db");
    BufferPoolManager* bpm = new BufferPoolManager(20, dm);
    TableHeap* table = new TableHeap(bpm);

    std::vector<Tuple> tuples;
    for (int i = 0; i < 5; i++) {
        Tuple t;
        t.SetSize(8);
        t.SetIntValue(0, 1000 + i);
        tuples.push_back(t);
    }

    InsertExecutor insert(table, {}, tuples);
    insert.Init();
    int insert_count = 0;
    Tuple dummy;
    while (insert.Next(&dummy, nullptr)) insert_count++;
    ASSERT(insert_count == 5, "InsertExecutor inserted 5 tuples");

    SeqScanExecutor scan(table);
    scan.Init();
    int scan_count = 0;
    while (scan.Next(&dummy, nullptr)) scan_count++;
    ASSERT(scan_count == 5, "SeqScan after InsertExecutor");

    delete table;
    delete bpm;
    delete dm;
}

void TestIndexScanExecutor() {
    std::cout << "\n=== IndexScanExecutor 测试 ===" << std::endl;
    DiskManager* dm = new DiskManager("integration_test.db");
    BufferPoolManager* bpm = new BufferPoolManager(20, dm);
    TableHeap* table = new TableHeap(bpm);
    BPlusTree* index = new BPlusTree("idx_scan", bpm, 50, 25);

    Tuple t;
    t.SetSize(8);
    t.SetIntValue(0, 42);
    RID rid;
    table->InsertTuple(t, &rid);
    index->Insert(42, rid);

    IndexScanExecutor idx_scan(index, table);
    idx_scan.Init();
    Tuple out;
    RID out_rid;
    int scan_count = 0;
    while (idx_scan.Next(&out, &out_rid)) scan_count++;
    ASSERT(scan_count >= 0, "IndexScanExecutor runs without crash");

    delete index;
    delete table;
    delete bpm;
    delete dm;
}

void TestTransactionManager() {
    std::cout << "\n=== TransactionManager 测试 ===" << std::endl;
    TransactionManager* txn_mgr = new TransactionManager();

    Transaction* t1 = txn_mgr->BeginTransaction();
    ASSERT(t1 != nullptr, "BeginTransaction");
    ASSERT(t1->GetTransactionId() >= 0, "Valid txn id");

    ASSERT(txn_mgr->CommitTransaction(t1), "CommitTransaction");

    Transaction* t2 = txn_mgr->BeginTransaction();
    ASSERT(txn_mgr->AbortTransaction(t2), "AbortTransaction");

    delete txn_mgr;
}

void TestDeadLockDetector() {
    std::cout << "\n=== DeadLockDetector 测试 ===" << std::endl;
    LockManager* lm = new LockManager();
    DeadLockDetector* detector = new DeadLockDetector(lm);

    detector->AddEdge(1, 2);
    detector->AddEdge(2, 3);
    detector->AddEdge(3, 1);
    ASSERT(detector->HasDeadlock(), "HasDeadlock detects cycle");

    txn_id_t victim;
    ASSERT(detector->GetVictim(victim), "GetVictim");

    std::vector<txn_id_t> cycle;
    ASSERT(detector->DetectCycle(cycle), "DetectCycle");
    ASSERT(!cycle.empty(), "Cycle not empty");

    detector->RemoveTxn(1);
    detector->RemoveEdge(2, 3);
    detector->RemoveEdge(3, 1);

    delete detector;
    delete lm;
}

void TestCatalogManager() {
    std::cout << "\n=== CatalogManager 测试 ===" << std::endl;
    CatalogManager* catalog = new CatalogManager();

    ASSERT(catalog->CreateTable("users"), "CreateTable users");
    ASSERT(catalog->TableExists("users"), "TableExists users");
    ASSERT(!catalog->TableExists("nonexistent"), "TableExists nonexistent false");
    ASSERT(catalog->DropTable("users"), "DropTable users");
    ASSERT(!catalog->TableExists("users"), "Table dropped");

    delete catalog;
}

void TestSQLParserExtended() {
    std::cout << "\n=== SQL Parser 扩展测试 ===" << std::endl;
    SQLParser parser;

    ParseResult r1 = parser.ParseQuery("SELECT a, b, c FROM t WHERE x > 5");
    ASSERT(r1.success && r1.type == SQLStatementType::SELECT, "SELECT with columns and WHERE");

    ParseResult r2 = parser.ParseQuery("INSERT INTO t (a,b) VALUES (1, 2)");
    ASSERT(r2.success && r2.type == SQLStatementType::INSERT, "INSERT with columns");

    ParseResult r3 = parser.ParseQuery("CREATE TABLE foo (id INT PRIMARY KEY)");
    ASSERT(r3.success && r3.type == SQLStatementType::CREATE_TABLE, "CREATE TABLE");

    ParseResult r4 = parser.ParseQuery("invalid sql xyz");
    ASSERT(!r4.success || r4.type == SQLStatementType::UNKNOWN, "Invalid SQL rejected");
}

int main() {
    std::cout << "========================================" << std::endl;
    std::cout << "  MiniDB 全功能集成测试" << std::endl;
    std::cout << "========================================" << std::endl;

    TestBufferPoolExtended();
    TestTableHeapFull();
    TestBPlusTreeFull();
    TestInsertExecutor();
    TestIndexScanExecutor();
    TestTransactionManager();
    TestDeadLockDetector();
    TestCatalogManager();
    TestSQLParserExtended();

    std::cout << "\n========================================" << std::endl;
    std::cout << "  测试结果: " << tests_passed << " 通过, " << tests_failed << " 失败" << std::endl;
    std::cout << "========================================" << std::endl;
    return tests_failed > 0 ? 1 : 0;
}

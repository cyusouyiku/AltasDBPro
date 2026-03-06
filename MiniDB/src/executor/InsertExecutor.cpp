//
//  InsertExecutor.cpp
//  MiniDB
//
//  Created by 张宗煜 on 2026/1/10.
//

#include "InsertExecutor.h"
#include "table_heap.h"
#include "BPlusTree.h"
#include "types.h"
#include "config.h"

InsertExecutor::InsertExecutor(
    TableHeap *table,
    std::vector<BPlusTree *> indexes,
    std::unique_ptr<Executor> child_executor)
    : table_(table),
      indexes(indexes),
      child_executor(std::move(child_executor)),
      direct_insert_index(0),
      initialized(false),
      finished(false),
      batch_count_(0) {
}

InsertExecutor::InsertExecutor(
    TableHeap *table,
    std::vector<BPlusTree *> indexes,
    const std::vector<Tuple> &tuples)
    : table_(table),
      indexes(indexes),
      child_executor(nullptr),
      direct_insert_tuples(tuples),
      direct_insert_index(0),
      initialized(false),
      finished(false),
      batch_count_(0) {
}

void InsertExecutor::Init() {
    if (child_executor != nullptr) {
        child_executor->Init();
    }
    initialized = true;
    finished = false;
}

bool InsertExecutor::Next(Tuple *tuple, RID *rid) {
    if (!initialized) {
        Init();
    }

    if (finished) {
        return false;
    }

    Tuple insert_tuple;
    bool has_tuple = false;

    if (child_executor != nullptr) {
        has_tuple = child_executor->Next(&insert_tuple, nullptr);
    } else {
        if (direct_insert_index < direct_insert_tuples.size()) {
            insert_tuple = direct_insert_tuples[direct_insert_index];
            direct_insert_index++;
            has_tuple = true;
        }
    }

    if (!has_tuple) {
        finished = true;
        return false;
    }

    RID inserted_rid;
    if (!table_->InsertTuple(insert_tuple, &inserted_rid)) {
        return false;
    }

    // 热数据批量攒批：每 BATCH_INSERT_SIZE 行一次性刷盘
    batch_count_++;
    if (batch_count_ >= BATCH_INSERT_SIZE) {
        table_->FlushBatch();
        batch_count_ = 0;
    }

    for (auto *index : indexes) {
        int key = ExtractKeyFromTuple(insert_tuple, index);
        index->Insert(key, inserted_rid);
    }

    if (tuple != nullptr) {
        *tuple = insert_tuple;
    }
    if (rid != nullptr) {
        *rid = inserted_rid;
    }

    return true;
}

int InsertExecutor::ExtractKeyFromTuple(const Tuple &tuple, BPlusTree *index) {
    (void)index;
    return tuple.GetIntValue(0);
}

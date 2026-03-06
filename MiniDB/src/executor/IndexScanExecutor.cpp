//
//  IndexScanExecutor.cpp
//  MiniDB
//
//  Created by 张宗煜 on 2026/1/10.
//

#include "IndexScanExecutor.h"
#include "../index/BPlusTree.h"
#include "../index/index_iterator.h"
#include "../storage/table_heap.h"
#include "../common/types.h"

IndexScanExecutor::IndexScanExecutor(BPlusTree *index, TableHeap *table)
    : index_(index), table_(table), initialized_(false) {
}

IndexScanExecutor::IndexScanExecutor(BPlusTree *index, TableHeap *table, const int &start_key, bool start_inclusive)
    : index_(index), table_(table), initialized_(false) {
    // 保存start_key和start_inclusive供Init使用
    (void)start_key;
    (void)start_inclusive;
}

void IndexScanExecutor::Init() {
    index_iter_ = index_->Begin();
    index_end_ = index_->End();
    initialized_ = true;
}

bool IndexScanExecutor::Next(Tuple *tuple, RID *rid) {
    if (!initialized_) {
        Init();
    }
    
    if (index_iter_ == index_end_) {
        return false;
    }
    
    RID current_rid = index_iter_.GetRID();
    if (!table_->GetTuple(current_rid, tuple)) {
        return false;
    }
    
    if (rid != nullptr) {
        *rid = current_rid;
    }
    ++index_iter_;
    return true;
}

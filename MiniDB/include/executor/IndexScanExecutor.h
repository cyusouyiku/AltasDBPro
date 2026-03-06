//
//  IndexScanExecutor.h
//  MiniDB
//
//  Created by 张宗煜 on 2026/1/10.
//

#ifndef INDEXSCANEXECUTOR_HPP
#define INDEXSCANEXECUTOR_HPP

#include "executor.h"
#include "BPlusTree.h"
#include "table_heap.h"
#include "index_iterator.h"

class IndexScanExecutor : public Executor {
private:
    BPlusTree *index_;
    TableHeap *table_;
    IndexIterator index_iter_;
    IndexIterator index_end_;
    bool initialized_;

public:
    IndexScanExecutor(BPlusTree *index, TableHeap *table);
    IndexScanExecutor(BPlusTree *index, TableHeap *table, const int &start_key, bool start_inclusive);
    ~IndexScanExecutor() override = default;
    void Init() override;
    bool Next(Tuple *tuple, RID *rid) override;
};

#endif

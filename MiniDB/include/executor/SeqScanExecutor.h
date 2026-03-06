//
//  SeqScanExecutor.h
//  MiniDB
//
//  Created by 张宗煜 on 2026/1/10.
//

#ifndef SEQSCANEXECUTOR_HPP
#define SEQSCANEXECUTOR_HPP

#include "executor.h"
#include "table_heap.h"
#include "../optimizer/QueryOptimizer.h"

class LayerManager;

class SeqScanExecutor : public Executor {
private:
    TableHeap *table_;
    TableIterator iter_;
    bool initialized_;
    LayerManager* layer_manager_;   // 可选：分层过滤
    DataLayer target_layer_;        // HOT/COLD/BOTH，BOTH 表示不过滤

public:
    SeqScanExecutor(TableHeap *table);
    SeqScanExecutor(TableHeap *table, LayerManager* layer_mgr, DataLayer target);
    ~SeqScanExecutor() override = default;
    void Init() override;
    bool Next(Tuple *tuple, RID *rid) override;
};

#endif

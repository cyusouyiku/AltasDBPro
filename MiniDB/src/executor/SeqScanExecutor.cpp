//
//  SeqScanExecutor.cpp
//  MiniDB
//
//  Created by 张宗煜 on 2026/1/10.
//

#include "SeqScanExecutor.h"
#include "table_heap.h"
#include "LayerManager.h"
#include "types.h"

SeqScanExecutor::SeqScanExecutor(TableHeap *table)
    : table_(table), iter_(table->Begin()), initialized_(true),
      layer_manager_(nullptr), target_layer_(DataLayer::BOTH) {
}

SeqScanExecutor::SeqScanExecutor(TableHeap *table, LayerManager* layer_mgr, DataLayer target)
    : table_(table), iter_(table->Begin()), initialized_(true),
      layer_manager_(layer_mgr), target_layer_(target) {
}

void SeqScanExecutor::Init() {
    iter_ = table_->Begin();
    initialized_ = true;
}

bool SeqScanExecutor::Next(Tuple *tuple, RID *rid) {
    if (!initialized_) {
        Init();
    }

    while (iter_ != table_->End()) {
        *tuple = *iter_;
        RID r = iter_.GetRID();
        ++iter_;

        if (layer_manager_ != nullptr && target_layer_ != DataLayer::BOTH) {
            bool page_hot = layer_manager_->IsPageHot(r.page_id);
            if (target_layer_ == DataLayer::HOT && !page_hot) continue;
            if (target_layer_ == DataLayer::COLD && page_hot) continue;
        }

        if (rid != nullptr) *rid = r;
        return true;
    }
    return false;
}

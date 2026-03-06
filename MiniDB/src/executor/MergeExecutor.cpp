//
//  MergeExecutor.cpp
//  MiniDB - 热层+冷层结果合并实现
//

#include "MergeExecutor.h"

MergeExecutor::MergeExecutor(std::unique_ptr<Executor> hot_executor,
                             std::unique_ptr<Executor> cold_executor)
    : hot_executor_(std::move(hot_executor)),
      cold_executor_(std::move(cold_executor)),
      use_hot_(true),
      hot_done_(false),
      cold_done_(false),
      initialized_(false) {
}

void MergeExecutor::Init() {
    if (hot_executor_) hot_executor_->Init();
    if (cold_executor_) cold_executor_->Init();
    use_hot_ = true;
    hot_done_ = false;
    cold_done_ = false;
    initialized_ = true;
}

bool MergeExecutor::Next(Tuple* tuple, RID* rid) {
    if (!initialized_) Init();

    // 先扫描热层，再扫描冷层（可扩展为真正并行）
    if (use_hot_ && hot_executor_) {
        if (hot_executor_->Next(tuple, rid)) {
            return true;
        }
        hot_done_ = true;
        use_hot_ = false;
    }
    if (cold_executor_) {
        if (cold_executor_->Next(tuple, rid)) {
            return true;
        }
        cold_done_ = true;
    }
    return false;
}

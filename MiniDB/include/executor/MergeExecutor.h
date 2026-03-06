//
//  MergeExecutor.h
//  MiniDB - 热层+冷层并行扫描、结果合并
//

#ifndef MERGEEXECUTOR_H
#define MERGEEXECUTOR_H

#include "executor.h"
#include <memory>

class MergeExecutor : public Executor {
private:
    std::unique_ptr<Executor> hot_executor_;
    std::unique_ptr<Executor> cold_executor_;
    bool use_hot_;   // true=从hot取, false=从cold取
    bool hot_done_;
    bool cold_done_;
    bool initialized_;

public:
    MergeExecutor(std::unique_ptr<Executor> hot_executor,
                  std::unique_ptr<Executor> cold_executor);
    ~MergeExecutor() override = default;
    void Init() override;
    bool Next(Tuple* tuple, RID* rid) override;
};

#endif // MERGEEXECUTOR_H

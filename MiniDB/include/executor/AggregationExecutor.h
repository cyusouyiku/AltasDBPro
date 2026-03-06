//
//  AggregationExecutor.h
//  MiniDB
//

#ifndef AGGREGATIONEXECUTOR_H
#define AGGREGATIONEXECUTOR_H

#include "executor.h"
#include "common/types.h"
#include <memory>

enum class AggregationType { COUNT, SUM, AVG, MIN, MAX };

class AggregationExecutor : public Executor {
private:
    std::unique_ptr<Executor> child_;
    AggregationType agg_type_;
    size_t key_offset_;
    bool done_;
    bool initialized_;
    int64_t count_;
    int64_t sum_;
    int min_val_;
    int max_val_;
    bool has_value_;

public:
    AggregationExecutor(std::unique_ptr<Executor> child, AggregationType agg_type, size_t key_offset = 0);
    ~AggregationExecutor() override = default;

    void Init() override;
    bool Next(Tuple* tuple, RID* rid) override;
};

#endif

//
//  SortExecutor.h
//  MiniDB
//

#ifndef SORTEXECUTOR_H
#define SORTEXECUTOR_H

#include "executor.h"
#include "common/types.h"
#include <memory>
#include <vector>

class SortExecutor : public Executor {
private:
    std::unique_ptr<Executor> child_;
    std::vector<Tuple> sorted_;
    size_t cursor_;
    size_t key_offset_;

public:
    SortExecutor(std::unique_ptr<Executor> child, size_t key_offset = 0);
    ~SortExecutor() override = default;

    void Init() override;
    bool Next(Tuple* tuple, RID* rid) override;
};

#endif

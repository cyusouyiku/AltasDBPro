//
//  JoinExecutor.h
//  MiniDB
//

#ifndef JOINEXECUTOR_H
#define JOINEXECUTOR_H

#include "executor.h"
#include "common/types.h"
#include <memory>

class JoinExecutor : public Executor {
private:
    std::unique_ptr<Executor> left_;
    std::unique_ptr<Executor> right_;
    Tuple left_current_;
    bool left_has_current_;
    bool initialized_;

public:
    explicit JoinExecutor(std::unique_ptr<Executor> left, std::unique_ptr<Executor> right);
    ~JoinExecutor() override = default;

    void Init() override;
    bool Next(Tuple* tuple, RID* rid) override;
};

#endif

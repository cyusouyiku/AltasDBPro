//
//  executor.h
//  MiniDB
//
//  Created by 张宗煜 on 2026/1/9.
//

#ifndef EXECUTOR_HPP
#define EXECUTOR_HPP

class Tuple;
struct RID;

class Executor {
public:
    Executor() = default;
    virtual ~Executor() = default;
    virtual void Init() = 0;
    virtual bool Next(Tuple *tuple, RID *rid) = 0;
};

#endif

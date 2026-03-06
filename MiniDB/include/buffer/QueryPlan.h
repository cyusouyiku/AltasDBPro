//
//  QueryPlan.h
//  MiniDB
//
//  Created by Auto-generated on 2026/1/24.
//

#ifndef QUERYPLAN_H
#define QUERYPLAN_H

#include <string>

// 查询计划基类
class QueryPlan {
protected:
    std::string query_string_;

public:
    explicit QueryPlan(const std::string& query) : query_string_(query) {}
    virtual ~QueryPlan() = default;

    virtual void Execute() = 0;
    virtual double GetCost() const = 0;

    const std::string& GetQueryString() const {
        return query_string_;
    }
};

#endif // QUERYPLAN_H
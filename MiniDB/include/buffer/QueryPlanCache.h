//
//  QueryPlanCache.h
//  MiniDB
//
//  Created by 张宗煜 on 2026/1/24.
//

#ifndef QUERYPLANCACHE_H
#define QUERYPLANCACHE_H

#include "QueryPlan.h"
#include <string>
#include <cstddef>

class QueryPlanCache {
private:
    size_t max_size_;

public:
    explicit QueryPlanCache(size_t max_size);
    ~QueryPlanCache();

    bool GetPlan(const std::string& query_key, QueryPlan*& plan);
    void PutPlan(const std::string& query_key, QueryPlan* plan);
    void EvictPlans();
};

#endif // QUERYPLANCACHE_H

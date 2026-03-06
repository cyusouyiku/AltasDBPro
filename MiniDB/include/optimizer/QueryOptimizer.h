//
//  QueryOptimizer.h
//  MiniDB
//
//  Created by Framework Generator on 2026/1/24.
//

#ifndef QUERYOPTIMIZER_H
#define QUERYOPTIMIZER_H

#include <string>

// 目标数据层级：用于分层路由
enum class DataLayer {
    HOT,   // 仅热层
    COLD,  // 仅冷层
    BOTH   // 热+冷（默认）
};

// 查询优化器 - 支持时间条件解析与分层预判
class QueryOptimizer {
private:
    int hot_threshold_days_;

public:
    explicit QueryOptimizer(int hot_threshold_days = 3);
    ~QueryOptimizer();

    std::string OptimizeQuery(const std::string& query);

    // 解析 WHERE 子句中的时间条件，返回天数（如近3天返回3）
    int ParseTimeConditionDays(const std::string& query) const;

    // 根据时间条件预判目标数据层级
    DataLayer InferTargetLayer(const std::string& query) const;

    void SetHotThreshold(int days) { hot_threshold_days_ = days; }
    int GetHotThreshold() const { return hot_threshold_days_; }
};

#endif // QUERYOPTIMIZER_H
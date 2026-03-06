//
//  QueryOptimizer.cpp
//  MiniDB
//
//  Created by Framework Generator on 2026/1/24.
//

#include "QueryOptimizer.h"
#include <regex>
#include <cctype>

QueryOptimizer::QueryOptimizer(int hot_threshold_days)
    : hot_threshold_days_(hot_threshold_days) {
}

QueryOptimizer::~QueryOptimizer() = default;

int QueryOptimizer::ParseTimeConditionDays(const std::string& query) const {
    // 匹配 "last N days", "近 N 天", "created_at > now() - N day" 等模式
    std::regex last_days(R"((?:last|past|recent|近|最近)\s*(\d+)\s*(?:days?|天))",
                        std::regex_constants::icase);
    std::smatch m;
    if (std::regex_search(query, m, last_days) && m.size() > 1) {
        return std::stoi(m[1].str());
    }
    std::regex num_days(R"((\d+)\s*(?:days?|天))", std::regex_constants::icase);
    if (std::regex_search(query, m, num_days) && m.size() > 1) {
        return std::stoi(m[1].str());
    }
    return -1;
}

DataLayer QueryOptimizer::InferTargetLayer(const std::string& query) const {
    int days = ParseTimeConditionDays(query);
    if (days < 0) {
        return DataLayer::BOTH;  // 无法解析则扫描全表
    }
    if (days <= hot_threshold_days_) {
        return DataLayer::HOT;   // 仅热层
    }
    return DataLayer::COLD;       // 仅冷层（或 BOTH 若冷热混合）
}

std::string QueryOptimizer::OptimizeQuery(const std::string& query) {
    (void)query;
    return query;  // 占位符：实际可改写 SQL
}
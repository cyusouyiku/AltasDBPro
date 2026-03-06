//
//  MetricsManager.cpp
//  MiniDB - 冷热分层监控指标实现
//

#include "MetricsManager.h"
#include <sstream>

void MetricsManager::SetHotLayerPages(uint64_t n) {
    hot_layer_pages_.store(n);
}

void MetricsManager::SetColdLayerPages(uint64_t n) {
    cold_layer_pages_.store(n);
}

void MetricsManager::IncHotLayerPages(int delta) {
    hot_layer_pages_.fetch_add(delta);
}

void MetricsManager::IncColdLayerPages(int delta) {
    cold_layer_pages_.fetch_add(delta);
}

void MetricsManager::RecordCacheHit() {
    cache_hits_.fetch_add(1);
}

void MetricsManager::RecordCacheMiss() {
    cache_misses_.fetch_add(1);
}

double MetricsManager::GetCacheHitRate() const {
    uint64_t hits = cache_hits_.load();
    uint64_t misses = cache_misses_.load();
    uint64_t total = hits + misses;
    return total > 0 ? static_cast<double>(hits) / total : 0.0;
}

void MetricsManager::RecordMigrationSuccess() {
    migration_success_count_.fetch_add(1);
}

void MetricsManager::RecordMigrationFail() {
    migration_fail_count_.fetch_add(1);
}

uint64_t MetricsManager::GetMigrationSuccessCount() const {
    return migration_success_count_.load();
}

uint64_t MetricsManager::GetMigrationFailCount() const {
    return migration_fail_count_.load();
}

void MetricsManager::RecordQueryLatency(uint64_t microseconds) {
    query_count_.fetch_add(1);
    query_total_us_.fetch_add(microseconds);
}

double MetricsManager::GetAvgQueryLatencyUs() const {
    uint64_t cnt = query_count_.load();
    return cnt > 0 ? static_cast<double>(query_total_us_.load()) / cnt : 0.0;
}

std::string MetricsManager::ExportPrometheusFormat() const {
    std::ostringstream oss;
    oss << "# HELP minidb_hot_layer_pages Hot layer page count\n";
    oss << "# TYPE minidb_hot_layer_pages gauge\n";
    oss << "minidb_hot_layer_pages " << hot_layer_pages_.load() << "\n";

    oss << "# HELP minidb_cold_layer_pages Cold layer page count\n";
    oss << "# TYPE minidb_cold_layer_pages gauge\n";
    oss << "minidb_cold_layer_pages " << cold_layer_pages_.load() << "\n";

    oss << "# HELP minidb_cache_hit_rate Buffer pool cache hit rate\n";
    oss << "# TYPE minidb_cache_hit_rate gauge\n";
    oss << "minidb_cache_hit_rate " << GetCacheHitRate() << "\n";

    oss << "# HELP minidb_cache_hits Total cache hits\n";
    oss << "# TYPE minidb_cache_hits counter\n";
    oss << "minidb_cache_hits " << cache_hits_.load() << "\n";

    oss << "# HELP minidb_cache_misses Total cache misses\n";
    oss << "# TYPE minidb_cache_misses counter\n";
    oss << "minidb_cache_misses " << cache_misses_.load() << "\n";

    oss << "# HELP minidb_migration_success Migration success count\n";
    oss << "# TYPE minidb_migration_success counter\n";
    oss << "minidb_migration_success " << migration_success_count_.load() << "\n";

    oss << "# HELP minidb_migration_fail Migration fail count\n";
    oss << "# TYPE minidb_migration_fail counter\n";
    oss << "minidb_migration_fail " << migration_fail_count_.load() << "\n";

    oss << "# HELP minidb_avg_query_latency_us Average query latency in microseconds\n";
    oss << "# TYPE minidb_avg_query_latency_us gauge\n";
    oss << "minidb_avg_query_latency_us " << GetAvgQueryLatencyUs() << "\n";

    return oss.str();
}

void MetricsManager::Reset() {
    hot_layer_pages_.store(0);
    cold_layer_pages_.store(0);
    cache_hits_.store(0);
    cache_misses_.store(0);
    migration_success_count_.store(0);
    migration_fail_count_.store(0);
    query_count_.store(0);
    query_total_us_.store(0);
}

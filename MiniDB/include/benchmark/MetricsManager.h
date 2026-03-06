//
//  MetricsManager.h
//  MiniDB - 冷热分层监控指标（Prometheus 兼容）
//

#ifndef METRICSMANAGER_H
#define METRICSMANAGER_H

#include <string>
#include <atomic>
#include <mutex>
#include <chrono>

class MetricsManager {
private:
    std::atomic<uint64_t> hot_layer_pages_{0};
    std::atomic<uint64_t> cold_layer_pages_{0};
    std::atomic<uint64_t> cache_hits_{0};
    std::atomic<uint64_t> cache_misses_{0};
    std::atomic<uint64_t> migration_success_count_{0};
    std::atomic<uint64_t> migration_fail_count_{0};
    std::atomic<uint64_t> query_count_{0};
    std::atomic<uint64_t> query_total_us_{0};
    std::mutex latch_;

public:
    MetricsManager() = default;
    ~MetricsManager() = default;

    // 分层数据量
    void SetHotLayerPages(uint64_t n);
    void SetColdLayerPages(uint64_t n);
    void IncHotLayerPages(int delta = 1);
    void IncColdLayerPages(int delta = 1);

    // 缓存命中率
    void RecordCacheHit();
    void RecordCacheMiss();
    double GetCacheHitRate() const;

    // 迁移统计
    void RecordMigrationSuccess();
    void RecordMigrationFail();
    uint64_t GetMigrationSuccessCount() const;
    uint64_t GetMigrationFailCount() const;

    // 查询耗时
    void RecordQueryLatency(uint64_t microseconds);
    double GetAvgQueryLatencyUs() const;

    // Prometheus 格式输出
    std::string ExportPrometheusFormat() const;

    // 重置
    void Reset();
};

#endif // METRICSMANAGER_H

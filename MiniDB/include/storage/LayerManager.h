//
//  LayerManager.h
//  MiniDB - 冷热分层管理器
//  负责数据迁移、分层规则、限流
//

#ifndef LAYERMANAGER_H
#define LAYERMANAGER_H

#include <string>
#include <unordered_set>
#include <mutex>
#include <chrono>
#include <vector>

class WALManager;

struct MigrationLogEntry {
    std::chrono::system_clock::time_point timestamp;
    int row_count;
    double duration_sec;
    bool success;
    std::string reason;
    std::string table_name;
};

class LayerManager {
private:
    std::unordered_set<int> hot_page_ids_;   // 热数据页集合
    int hot_threshold_days_;                 // 热数据时间阈值（天）
    int migration_rate_limit_;               // 迁移限流（行/秒）
    std::chrono::steady_clock::time_point last_migration_time_;
    int rows_migrated_this_second_;
    mutable std::mutex latch_;
    WALManager* wal_manager_;
    std::vector<MigrationLogEntry> migration_logs_;

public:
    explicit LayerManager(WALManager* wal = nullptr);
    ~LayerManager() = default;

    // 分层规则
    void SetHotThreshold(int days);
    int GetHotThreshold() const;
    void SetMigrationRateLimit(int rows_per_sec);
    int GetMigrationRateLimit() const;

    // 页层级判断
    bool IsPageHot(int page_id) const;
    void MarkPageHot(int page_id);
    void MarkPageCold(int page_id);

    // 迁移（带限流）
    bool CanMigrateMore();
    void RecordMigrationStart();
    void RecordMigrationRows(int rows);
    void LogMigration(const std::string& table_name, int row_count,
                     double duration_sec, bool success, const std::string& reason);

    // 迁移日志查询
    std::vector<MigrationLogEntry> GetMigrationLogs() const;
    void ClearMigrationLogs();

    void SetWALManager(WALManager* wal) { wal_manager_ = wal; }
};

#endif // LAYERMANAGER_H

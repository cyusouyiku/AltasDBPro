//
//  LayerManager.cpp
//  MiniDB - 冷热分层管理器实现
//

#include "LayerManager.h"
#include "WALManager.h"
#include "../common/config.h"
#include <sstream>

LayerManager::LayerManager(WALManager* wal)
    : hot_threshold_days_(HOT_THRESHOLD_DAYS),
      migration_rate_limit_(MIGRATION_RATE_LIMIT),
      rows_migrated_this_second_(0),
      wal_manager_(wal) {
}

void LayerManager::SetHotThreshold(int days) {
    std::lock_guard<std::mutex> lock(latch_);
    hot_threshold_days_ = days;
}

int LayerManager::GetHotThreshold() const {
    return hot_threshold_days_;
}

void LayerManager::SetMigrationRateLimit(int rows_per_sec) {
    std::lock_guard<std::mutex> lock(latch_);
    migration_rate_limit_ = rows_per_sec;
}

int LayerManager::GetMigrationRateLimit() const {
    return migration_rate_limit_;
}

bool LayerManager::IsPageHot(int page_id) const {
    std::lock_guard<std::mutex> lock(latch_);
    return hot_page_ids_.count(page_id) != 0;
}

void LayerManager::MarkPageHot(int page_id) {
    std::lock_guard<std::mutex> lock(latch_);
    hot_page_ids_.insert(page_id);
}

void LayerManager::MarkPageCold(int page_id) {
    std::lock_guard<std::mutex> lock(latch_);
    hot_page_ids_.erase(page_id);
}

bool LayerManager::CanMigrateMore() {
    std::lock_guard<std::mutex> lock(latch_);
    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(
        now - last_migration_time_).count();
    if (elapsed >= 1) {
        rows_migrated_this_second_ = 0;
        last_migration_time_ = now;
    }
    return rows_migrated_this_second_ < migration_rate_limit_;
}

void LayerManager::RecordMigrationStart() {
    std::lock_guard<std::mutex> lock(latch_);
    last_migration_time_ = std::chrono::steady_clock::now();
}

void LayerManager::RecordMigrationRows(int rows) {
    std::lock_guard<std::mutex> lock(latch_);
    rows_migrated_this_second_ += rows;
}

void LayerManager::LogMigration(const std::string& table_name, int row_count,
                                double duration_sec, bool success,
                                const std::string& reason) {
    MigrationLogEntry entry;
    entry.timestamp = std::chrono::system_clock::now();
    entry.row_count = row_count;
    entry.duration_sec = duration_sec;
    entry.success = success;
    entry.reason = reason;
    entry.table_name = table_name;

    {
        std::lock_guard<std::mutex> lock(latch_);
        migration_logs_.push_back(entry);
        if (migration_logs_.size() > 10000) {
            migration_logs_.erase(migration_logs_.begin());
        }
    }

    if (wal_manager_ != nullptr) {
        std::ostringstream oss;
        oss << "MIGRATION|" << table_name << "|" << row_count << "|"
            << duration_sec << "|" << (success ? "OK" : "FAIL") << "|" << reason;
        wal_manager_->WriteLog(oss.str());
    }
}

std::vector<MigrationLogEntry> LayerManager::GetMigrationLogs() const {
    std::lock_guard<std::mutex> lock(latch_);
    return migration_logs_;
}

void LayerManager::ClearMigrationLogs() {
    std::lock_guard<std::mutex> lock(latch_);
    migration_logs_.clear();
}

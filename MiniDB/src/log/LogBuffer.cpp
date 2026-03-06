//
//  LogBuffer.cpp
//  MiniDB
//

#include "LogBuffer.h"
#include "WALManager.h"

LogBuffer::LogBuffer(size_t buffer_size, WALManager* wal) : buffer_size_(buffer_size), wal_(wal) {}

LogBuffer::~LogBuffer() {
    Flush();
}

bool LogBuffer::AppendLog(const LogRecord& record) {
    records_.push_back(record);
    if (records_.size() >= buffer_size_ && buffer_size_ > 0) Flush();
    return true;
}

void LogBuffer::Flush() {
    if (!wal_ || records_.empty()) return;
    for (const auto& r : records_) {
        std::string entry;
        entry.push_back(static_cast<char>(r.type));
        entry.append(reinterpret_cast<const char*>(&r.txn_id), sizeof(r.txn_id));
        entry.append(reinterpret_cast<const char*>(&r.lsn), sizeof(r.lsn));
        entry.append(r.data, 64);
        wal_->WriteLog(entry);
    }
    wal_->FlushLogs();
    records_.clear();
}

size_t LogBuffer::GetSize() const {
    return buffer_size_;
}
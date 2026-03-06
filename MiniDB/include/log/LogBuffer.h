//
//  LogBuffer.h
//  MiniDB
//
//  Created by 张宗煜 on 2026/1/24.
//

#ifndef LOGBUFFER_H
#define LOGBUFFER_H

#include "LogRecord.h"
#include <cstddef>
#include <vector>
#include <string>

class WALManager;

class LogBuffer {
private:
    size_t buffer_size_;
    std::vector<LogRecord> records_;
    WALManager* wal_;

public:
    explicit LogBuffer(size_t buffer_size, WALManager* wal = nullptr);
    ~LogBuffer();

    bool AppendLog(const LogRecord& record);
    void Flush();
    size_t GetSize() const;
    void SetWAL(WALManager* wal) { wal_ = wal; }
};

#endif
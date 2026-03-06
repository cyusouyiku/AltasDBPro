//
//  LogRecord.h
//  MiniDB
//
//  Created by Auto-generated on 2026/1/24.
//

#ifndef LOGRECORD_H
#define LOGRECORD_H

#include <cstdint>
#include <cstring>

enum class LogRecordType {
    INSERT,
    UPDATE,
    DELETE,
    COMMIT,
    ABORT
};

struct LogRecord {
    LogRecordType type;
    int txn_id;
    uint64_t lsn;  // Log Sequence Number
    char data[1024]; // 简化实现，实际应该更复杂

    LogRecord(LogRecordType t, int id) : type(t), txn_id(id), lsn(0) {
        memset(data, 0, sizeof(data));
    }
};

#endif // LOGRECORD_H
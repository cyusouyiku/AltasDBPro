//
//  LockTable.h
//  MiniDB
//
//  Created by 张宗煜 on 2026/1/24.
//

#ifndef LOCKTABLE_H
#define LOCKTABLE_H

#include "common/rid.h"

class Transaction;

enum class LockMode {
    SHARED,
    EXCLUSIVE
};

class LockTable {
public:
    LockTable();
    ~LockTable();

    bool GrantLock(Transaction* txn, const RID& rid, LockMode mode);
    bool ReleaseLock(Transaction* txn, const RID& rid);
    bool DetectDeadlock();
};

#endif // LOCKTABLE_H
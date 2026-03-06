//
//  LockManager.h
//  MiniDB
//
//  Created by 张宗煜 on 2026/1/24.
//



#ifndef LOCKMANAGER_HPP
#define LOCKMANAGER_HPP

#include "Transaction.h"
#include "rid.h"
#include <mutex>
#include <unordered_map>
#include <unordered_set>
#include <queue>
#include <string>

enum class LockType {
    NONE,
    SHARED,
    EXCLUSIVE
};

struct LockInfo {
    LockType lock_type;
    std::unordered_set<Transaction*> holders;  // 持有该锁的事务集合
    std::queue<std::pair<Transaction*, LockType>> wait_queue;  // 等待队列
    
    LockInfo() : lock_type(LockType::NONE) {}
};

class LockManager {
private:
    std::unordered_map<RID, LockInfo> lock_table_;
    mutable std::mutex mtx_;

public:
    LockManager();
    ~LockManager();
    std::string GetLockType(const RID& rid) const;
    bool LockShared(Transaction* txn, const RID& rid);
    bool LockExclusive(Transaction* txn, const RID& rid);
    bool Unlock(Transaction* txn, const RID& rid);
};

#endif // LOCKMANAGER_HPP

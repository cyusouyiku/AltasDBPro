//
//  LockManager.cpp
//  MiniDB
//
//  Created by Auto-generated on 2026/1/24.
//

#include "LockManager.h"
#include "Transaction.h"
#include <mutex>
#include <unordered_map>
#include <queue>

LockManager::LockManager() {}

LockManager::~LockManager() {}

std::string LockManager::GetLockType(const RID& rid) const {
    std::lock_guard<std::mutex> lock(mtx_);
    auto it = lock_table_.find(rid);
    if (it == lock_table_.end()) {
        return "NONE";
    }
    const LockInfo& info = it->second;
    switch (info.lock_type) {
        case LockType::SHARED:
            return "SHARED";
        case LockType::EXCLUSIVE:
            return "EXCLUSIVE";
        default:
            return "NONE";
    }
}

bool LockManager::LockShared(Transaction* txn, const RID& rid) {
    if (!txn) {
        return false;
    }
    Transaction* curr_txn = txn;
    bool is_held_by_others = false;
    std::lock_guard<std::mutex> lock(mtx_);
    LockInfo& lock_info = lock_table_[rid];
    if (lock_info.holders.count(curr_txn)) {
        return true;
    }
    if (!lock_info.holders.empty()) {
        for (Transaction* hold_txn : lock_info.holders) {
            if (hold_txn != curr_txn) {
                is_held_by_others = true;
                break;
            }
        }
    }
    if (is_held_by_others) {
        if (lock_info.lock_type == LockType::EXCLUSIVE) {
            return false;
        }
    }
    lock_info.lock_type = LockType::SHARED;
    lock_info.holders.insert(curr_txn);
    curr_txn->AddLock(rid);

    return true;
}

bool LockManager::LockExclusive(Transaction* txn, const RID& rid) {
    if (!txn) {
        return false;
    }
    Transaction* curr_txn = txn;
    bool is_held_by_others = false;
    std::lock_guard<std::mutex> lock(mtx_);
    LockInfo& lock_info = lock_table_[rid];
    if (lock_info.holders.count(curr_txn)) {
        return true;
    }
    if (!lock_info.holders.empty()) {
        for (Transaction* hold_txn : lock_info.holders) {
            if (hold_txn != curr_txn) {
                is_held_by_others = true;
                break;
            }
        }
    }
    if (is_held_by_others) {
        if (lock_info.lock_type == LockType::SHARED) {
            return false;
        }
    }
    lock_info.lock_type = LockType::EXCLUSIVE;
    lock_info.holders.insert(curr_txn);
    curr_txn->AddLock(rid);
    return true;
}

bool LockManager::Unlock(Transaction* txn, const RID& rid) {
    if (!txn) return false;

    std::lock_guard<std::mutex> lock(mtx_);
    auto it = lock_table_.find(rid);
    if (it != lock_table_.end()) {
        LockInfo& lock_info = it->second;
        lock_info.holders.erase(txn);
        if (lock_info.holders.empty()) {
            lock_info.lock_type = LockType::NONE;
        }
    }
    txn->RemoveLock(rid);
    return true;
}

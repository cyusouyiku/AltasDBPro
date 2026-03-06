//
//  TransactionManager.cpp
//  MiniDB
//
//  Created by Auto-generated on 2026/1/24.
//

#include "TransactionManager.h"

// TransactionManager 实现
TransactionManager::TransactionManager() : next_txn_id_(1) {
}

TransactionManager::~TransactionManager() {}

Transaction* TransactionManager::BeginTransaction() {
    std::lock_guard<std::mutex> lock(latch_);
    int txn_id = next_txn_id_++;
    auto* txn = new Transaction(txn_id);
    return txn;
}
bool TransactionManager::CommitTransaction(Transaction* txn) {
    std::lock_guard<std::mutex> lock(latch_);

    txn->SetState(TransactionState::COMMITTED);
    return true;
}
bool TransactionManager::AbortTransaction(Transaction* txn) {
    std::lock_guard<std::mutex> lock(latch_);
    txn->SetState(TransactionState::ABORTED);
    return true;
}

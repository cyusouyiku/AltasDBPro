//
//  Transaction.cpp
//  MiniDB
//
//  Created by Auto-generated on 2026/1/24.
//

#include "Transaction.h"

Transaction::Transaction(int txn_id)
    : txn_id_(txn_id), state_(TransactionState::RUNNING) {
}

int Transaction::GetTransactionId() const {
    return txn_id_;
}

TransactionState Transaction::GetState() const {
    return state_;
}

void Transaction::SetState(TransactionState state) {
    state_ = state;
}

bool Transaction::HasLock(const RID& rid) const {
    return locks_held_.find(rid) != locks_held_.end();
}

void Transaction::AddLock(const RID& rid) {
    locks_held_.insert(rid);
}

void Transaction::RemoveLock(const RID& rid) {
    locks_held_.erase(rid);
}

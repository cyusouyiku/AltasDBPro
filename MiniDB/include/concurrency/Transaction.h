//
//  Transaction.h
//  MiniDB
//
//  Created by Auto-generated on 2026/1/24.
//

#ifndef TRANSACTION_H
#define TRANSACTION_H

#include "common/rid.h"
#include <unordered_set>

using txn_id_t = int;

enum class TransactionState {
    RUNNING,
    COMMITTED,
    ABORTED
};

class Transaction {
private:
    int txn_id_;
    TransactionState state_;
    std::unordered_set<RID> locks_held_;

public:
    explicit Transaction(int txn_id);
    ~Transaction() = default;

    int GetTransactionId() const;
    TransactionState GetState() const;
    void SetState(TransactionState state);

    bool HasLock(const RID& rid) const;
    void AddLock(const RID& rid);
    void RemoveLock(const RID& rid);
};

#endif // TRANSACTION_H

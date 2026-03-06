//
//  TransactionManager.h
//  MiniDB
//
//  Created by Auto-generated on 2026/1/24.
//

#ifndef TRANSACTIONMANAGER_H
#define TRANSACTIONMANAGER_H

#include "Transaction.h"
#include <unordered_map>
#include <mutex>

class TransactionManager {
private:
    std::unordered_map<int, Transaction*> transactions_;
    std::mutex latch_;
    int next_txn_id_;

public:
    TransactionManager();
    ~TransactionManager();

    Transaction* BeginTransaction();
    bool CommitTransaction(Transaction* txn);
    bool AbortTransaction(Transaction* txn);

    Transaction* GetTransaction(Transaction* txn);
};

#endif // TRANSACTIONMANAGER_H

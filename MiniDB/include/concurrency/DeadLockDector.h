//
//  DeadLockDector.h
//  MiniDB
//
//  Created by 张宗煜 on 2026/1/28.
//

#ifndef DEADLOCKDETECTOR_HPP
#define DEADLOCKDETECTOR_HPP

#include "Transaction.h"
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <mutex>
#include "LockManager.h"

class DeadLockDetector {
private:
    LockManager* lock_manager_;
    mutable std::mutex mtx_;
    std::unordered_map<txn_id_t, std::unordered_set<txn_id_t>> wait_graph_;

public:
    DeadLockDetector(LockManager* lock_manager);
    ~DeadLockDetector();

    void AddEdge(txn_id_t from, txn_id_t to);
    void RemoveEdge(txn_id_t from, txn_id_t to);
    void RemoveTxn(txn_id_t txn);

    bool HasDeadlock() const;
    bool DetectCycle(std::vector<txn_id_t>& cycle) const;
    bool GetVictim(txn_id_t& victim) const;

private:
    bool DFSCycle(txn_id_t node,
                  std::unordered_set<txn_id_t>& visited,
                  std::unordered_set<txn_id_t>& rec_stack,
                  std::vector<txn_id_t>& path,
                  std::vector<txn_id_t>& cycle) const;
    
    void BuildWaitGraph();
};

#endif

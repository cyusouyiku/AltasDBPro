//
//  DeadLockDector.cpp
//  MiniDB
//
//  Created by 张宗煜 on 2026/1/28.
//
#include "Transaction.h"
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <mutex>
#include "LockManager.h"
#include <utility>
#include "DeadLockDector.h"

DeadLockDetector::DeadLockDetector(LockManager* lock_manager)
    : lock_manager_(lock_manager) {}

DeadLockDetector::~DeadLockDetector() {}

void DeadLockDetector::AddEdge(txn_id_t from, txn_id_t to) {
    std::lock_guard<std::mutex> lock(mtx_);
    wait_graph_[from].insert(to);
}

void DeadLockDetector::RemoveEdge(txn_id_t from, txn_id_t to) {
    std::lock_guard<std::mutex> lock(mtx_);
    auto it = wait_graph_.find(from);
    if (it != wait_graph_.end()) {
        it->second.erase(to);
        if (it->second.empty()) {
            wait_graph_.erase(it);
        }
    }
}

void DeadLockDetector::RemoveTxn(txn_id_t txn) {
    std::lock_guard<std::mutex> lock(mtx_);
    
    wait_graph_.erase(txn);
    
    for (auto& pair : wait_graph_) {
        pair.second.erase(txn);
    }
    
    auto it = wait_graph_.begin();
    while (it != wait_graph_.end()) {
        if (it->second.empty()) {
            it = wait_graph_.erase(it);
        } else {
            ++it;
        }
    }
}

bool DeadLockDetector::HasDeadlock() const {
    std::lock_guard<std::mutex> lock(mtx_);
    
    std::unordered_set<txn_id_t> visited;
    std::unordered_set<txn_id_t> rec_stack;
    std::vector<txn_id_t> path;
    std::vector<txn_id_t> cycle;
    
    for (const auto& pair : wait_graph_) {
        if (visited.find(pair.first) == visited.end()) {
            if (DFSCycle(pair.first, visited, rec_stack, path, cycle)) {
                return true;
            }
        }
    }
    
    return false;
}

bool DeadLockDetector::DFSCycle(txn_id_t node,
                                std::unordered_set<txn_id_t>& visited,
                                std::unordered_set<txn_id_t>& rec_stack,
                                std::vector<txn_id_t>& path,
                                std::vector<txn_id_t>& cycle) const {
    visited.insert(node);
    rec_stack.insert(node);
    path.push_back(node);
    auto it = wait_graph_.find(node);
    if (it != wait_graph_.end()) {
        for (txn_id_t next : it->second) {
            if (visited.find(next) == visited.end()) {
                if (DFSCycle(next, visited, rec_stack, path, cycle)) {
                    return true;
                }
            } else if (rec_stack.find(next) != rec_stack.end()) {
                cycle.clear();
                size_t idx = 0;
                for (; idx < path.size() && path[idx] != next; ++idx) {}
                for (; idx < path.size(); ++idx) {
                    cycle.push_back(path[idx]);
                }
                cycle.push_back(next);
                rec_stack.erase(node);
                path.pop_back();
                return true;
            }
        }
    }
    rec_stack.erase(node);
    path.pop_back();
    return false;
}

bool DeadLockDetector::DetectCycle(std::vector<txn_id_t>& cycle) const {
    std::lock_guard<std::mutex> lock(mtx_);
    std::unordered_set<txn_id_t> visited;
    std::unordered_set<txn_id_t> rec_stack;
    std::vector<txn_id_t> path;
    cycle.clear();
    for (const auto& pair : wait_graph_) {
        if (visited.find(pair.first) == visited.end()) {
            if (DFSCycle(pair.first, visited, rec_stack, path, cycle)) {
                return true;
            }
        }
    }
    return false;
}

bool DeadLockDetector::GetVictim(txn_id_t& victim) const {
    std::vector<txn_id_t> cycle;
    if (!DetectCycle(cycle) || cycle.empty()) {
        return false;
    }
    victim = cycle[0];
    for (txn_id_t t : cycle) {
        if (t > victim) victim = t;
    }
    return true;
}

void DeadLockDetector::BuildWaitGraph() {
    // 可由 LockManager 维护的等待关系同步到 wait_graph_，当前由外部 AddEdge/RemoveEdge 维护
}

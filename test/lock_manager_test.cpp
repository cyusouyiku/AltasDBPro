//
//  lock_manager_test.cpp
//  MiniDB
//
//  Created by Auto-generated on 2026/1/24.
//

#include "../MiniDB/include/concurrency/LockManager.h"
#include "../MiniDB/include/concurrency/Transaction.h"
#include "../MiniDB/include/common/rid.h"
#include <iostream>
#include <cassert>

// 锁管理器测试
void TestLockManager() {
    std::cout << "Testing LockManager..." << std::endl;

    LockManager* lm = new LockManager();
    Transaction* txn = new Transaction(1);

    RID rid(0, 0);
    bool success = lm->LockShared(txn, rid);
    assert(success);
    std::cout << "LockShared test passed" << std::endl;

    success = lm->LockExclusive(txn, rid);
    assert(success);
    std::cout << "LockExclusive test passed" << std::endl;

    success = lm->Unlock(txn, rid);
    assert(success);
    std::cout << "Unlock test passed" << std::endl;

    delete txn;
    delete lm;

    std::cout << "LockManager tests completed" << std::endl;
}

int main() {
    TestLockManager();
    return 0;
}
//
//  CheckpointManager.h
//  MiniDB
//
//  Created by Framework Generator on 2026/1/24.
//

#ifndef CHECKPOINTMANAGER_H
#define CHECKPOINTMANAGER_H

// 检查点管理器（规划中）
class CheckpointManager {
public:
    CheckpointManager();
    ~CheckpointManager();

    bool CreateCheckpoint();
    bool RecoverFromCheckpoint();
};

#endif // CHECKPOINTMANAGER_H
//
//  InsertExecutor.h
//  MiniDB
//
//  Created by 张宗煜 on 2026/1/10.
//

#ifndef INSERTEXECUTOR_HPP
#define INSERTEXECUTOR_HPP

#include "executor.h"
#include "../common/config.h"
#include <vector>
#include <memory>

class TableHeap;
class BPlusTree;
class Tuple;
struct RID;

class InsertExecutor : public Executor {
private:
    TableHeap *table_;
    std::vector<BPlusTree *> indexes;
    std::unique_ptr<Executor> child_executor;
    std::vector<Tuple> direct_insert_tuples;
    size_t direct_insert_index;
    bool initialized;
    bool finished;
    int batch_count_;  // 批量写入计数，每 BATCH_INSERT_SIZE 刷盘一次

public:
    InsertExecutor(TableHeap *table, std::vector<BPlusTree *> indexes, std::unique_ptr<Executor> child_executor);
    InsertExecutor(TableHeap *table, std::vector<BPlusTree *> indexes, const std::vector<Tuple> &tuples);
    ~InsertExecutor() override = default;
    void Init() override;
    bool Next(Tuple *tuple, RID *rid) override;

private:
    int ExtractKeyFromTuple(const Tuple &tuple, BPlusTree *index);
};

#endif

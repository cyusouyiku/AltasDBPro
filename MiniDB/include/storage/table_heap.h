//
//  table_heap.h
//  MiniDB
//
//  Created on 2026/1/10.
//

#ifndef TABLE_HEAP_H
#define TABLE_HEAP_H

#include "../common/types.h"
#include "../common/rid.h"
#include "../buffer/BufferPoolManager.h"

class BufferPoolManager;
class Page;

// Forward declaration for TableIterator (defined after TableHeap)
class TableIterator;

// TableHeap: 表堆存储
// 管理表中的数据存储，提供插入、删除、查找等操作
class TableHeap {
private:
    BufferPoolManager* buffer_pool_manager_;
    int first_page_id_;
    int last_page_id_;
    
public:
    TableHeap(BufferPoolManager *buffer_pool_manager);
    ~TableHeap();
    
    // 插入一条记录
    bool InsertTuple(const Tuple &tuple, RID *rid);
    
    // 根据RID获取记录
    bool GetTuple(const RID &rid, Tuple *tuple);
    
    // 删除记录
    bool DeleteTuple(const RID &rid);
    
    // 更新记录
    bool UpdateTuple(const Tuple &tuple, const RID &rid);
    
    // 获取迭代器（指向第一条记录）
    TableIterator Begin();
    
    // 获取迭代器（指向结束位置）
    TableIterator End();
    
    // 根据RID获取迭代器
    TableIterator GetIterator(const RID &rid);
    
    int GetFirstPageId() const { return first_page_id_; }
    int GetLastPageId() const { return last_page_id_; }
    int GetSlotCount(int page_id) const;

    // 批量刷盘：热数据攒批后一次性刷盘，减少 SSD 随机写
    void FlushBatch();
};

// TableIterator: 表迭代器 (defined after TableHeap for GetSlotCount access)
class TableIterator {
private:
    TableHeap *table_;
    RID current_rid_;
    bool is_end_;

public:
    TableIterator(TableHeap *table, const RID &rid, bool is_end = false)
        : table_(table), current_rid_(rid), is_end_(is_end) {}

    ~TableIterator() = default;

    TableIterator(const TableIterator &other)
        : table_(other.table_), current_rid_(other.current_rid_), is_end_(other.is_end_) {}

    TableIterator &operator=(const TableIterator &other) {
        if (this != &other) {
            table_ = other.table_;
            current_rid_ = other.current_rid_;
            is_end_ = other.is_end_;
        }
        return *this;
    }

    bool operator==(const TableIterator &other) const {
        if (is_end_ && other.is_end_ && table_ == other.table_) return true;
        return table_ == other.table_ && current_rid_ == other.current_rid_ && is_end_ == other.is_end_;
    }

    bool operator!=(const TableIterator &other) const {
        return !(*this == other);
    }

    TableIterator &operator++() {
        if (is_end_) return *this;
        current_rid_.slot_num++;
        if (table_) {
            int n = table_->GetSlotCount(current_rid_.page_id);
            if (current_rid_.slot_num >= n) is_end_ = true;
        }
        return *this;
    }

    Tuple operator*();

    RID GetRID() const {
        return current_rid_;
    }

    void SetEnd(bool is_end) {
        is_end_ = is_end;
    }
};

#endif // TABLE_HEAP_H

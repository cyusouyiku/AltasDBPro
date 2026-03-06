//
//  types.h
//  MiniDB
//
//  Created on 2026/1/10.
//

#ifndef TYPES_H
#define TYPES_H

#include "rid.h"
#include <cstring>

// Tuple: 数据库记录
// 简化实现：使用固定大小的字节数组存储记录数据
class Tuple {
public:
    static constexpr size_t MAX_SIZE = 2048;  // 最大记录大小
    
private:
    char data_[MAX_SIZE];
    size_t size_;
    RID rid_;
    
public:
    Tuple() : size_(0) {
        memset(data_, 0, MAX_SIZE);
    }
    
    Tuple(const char *data, size_t size, const RID &rid) 
        : size_(size), rid_(rid) {
        memset(data_, 0, MAX_SIZE);
        if (size <= MAX_SIZE) {
            memcpy(data_, data, size);
        }
    }
    
    ~Tuple() = default;
    
    Tuple(const Tuple &other) 
        : size_(other.size_), rid_(other.rid_) {
        memcpy(data_, other.data_, MAX_SIZE);
    }
    
    Tuple &operator=(const Tuple &other) {
        if (this != &other) {
            size_ = other.size_;
            rid_ = other.rid_;
            memcpy(data_, other.data_, MAX_SIZE);
        }
        return *this;
    }
    
    const char *GetData() const {
        return data_;
    }
    
    char *GetData() {
        return data_;
    }
    
    size_t GetSize() const {
        return size_;
    }
    
    void SetSize(size_t size) {
        if (size <= MAX_SIZE) {
            size_ = size;
        }
    }
    
    const RID &GetRID() const {
        return rid_;
    }
    
    RID &GetRID() {
        return rid_;
    }
    
    void SetRID(const RID &rid) {
        rid_ = rid;
    }
    
    // 从字节数组中提取整数值（用于索引键提取）
    int GetIntValue(size_t offset) const {
        if (offset + sizeof(int) <= size_) {
            int value;
            memcpy(&value, data_ + offset, sizeof(int));
            return value;
        }
        return 0;
    }
    
    // 设置整数值
    void SetIntValue(size_t offset, int value) {
        if (offset + sizeof(int) <= MAX_SIZE) {
            memcpy(data_ + offset, &value, sizeof(int));
            if (offset + sizeof(int) > size_) {
                size_ = offset + sizeof(int);
            }
        }
    }
};

#endif // TYPES_H

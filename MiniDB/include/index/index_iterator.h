//
//  index_iterator.h
//  MiniDB
//
//  Created on 2026/1/10.
//

#ifndef INDEX_ITERATOR_H
#define INDEX_ITERATOR_H

#include "../common/rid.h"

class BPlusTree;
class Page;

// IndexIterator: B+树索引迭代器
// 用于遍历索引条目，支持范围查询
class IndexIterator {
private:
    BPlusTree *b_plus_tree_;
    Page *current_leaf_page_;
    int current_key_index_;
    int current_page_id_;
    bool is_end_;
    
public:
    IndexIterator() 
        : b_plus_tree_(nullptr), current_leaf_page_(nullptr), 
          current_key_index_(-1), current_page_id_(-1), is_end_(true) {}
    
    IndexIterator(BPlusTree *tree, Page *leaf_page, int key_index, int page_id, bool is_end = false)
        : b_plus_tree_(tree), current_leaf_page_(leaf_page), 
          current_key_index_(key_index), current_page_id_(page_id), is_end_(is_end) {
        // 如果is_end为true，确保标记为结束
        if (is_end) {
            is_end_ = true;
        }
    }
    
    ~IndexIterator() = default;
    
    IndexIterator(const IndexIterator &other)
        : b_plus_tree_(other.b_plus_tree_),
          current_leaf_page_(other.current_leaf_page_),
          current_key_index_(other.current_key_index_),
          current_page_id_(other.current_page_id_),
          is_end_(other.is_end_) {}
    
    IndexIterator &operator=(const IndexIterator &other) {
        if (this != &other) {
            b_plus_tree_ = other.b_plus_tree_;
            current_leaf_page_ = other.current_leaf_page_;
            current_key_index_ = other.current_key_index_;
            current_page_id_ = other.current_page_id_;
            is_end_ = other.is_end_;
        }
        return *this;
    }
    
    bool operator==(const IndexIterator &other) const {
        return b_plus_tree_ == other.b_plus_tree_ &&
               current_page_id_ == other.current_page_id_ &&
               current_key_index_ == other.current_key_index_ &&
               is_end_ == other.is_end_;
    }
    
    bool operator!=(const IndexIterator &other) const {
        return !(*this == other);
    }
    
    IndexIterator &operator++() {
        // 移动到下一个索引条目
        // 简化实现：实际需要遍历叶子节点链表
        if (!is_end_ && current_leaf_page_ != nullptr) {
            current_key_index_++;
            // 如果超出当前页面的键数量，移动到下一个叶子页面
            // 这里需要实际的逻辑来遍历叶子节点
            const int* data = reinterpret_cast<const int*>(current_leaf_page_->GetData());
            int size = data[1];
            if (current_key_index_ >= size) {
                // 移动到下一个叶子页面（简化处理）
                int next_page_id = data[2 + size + size * 2];  // next_leaf_page_id
                if (next_page_id >= 0) {
                    current_page_id_ = next_page_id;
                    current_key_index_ = 0;
                    // 这里应该重新FetchPage，简化处理
                } else {
                    is_end_ = true;
                }
            }
        }
        return *this;
    }
    
    // 获取当前索引条目的键值
    int GetKey() const {
        if (is_end_ || current_leaf_page_ == nullptr || current_key_index_ < 0) {
            return 0;
        }
        const int* data = reinterpret_cast<const int*>(current_leaf_page_->GetData());
        int size = data[1];
        if (current_key_index_ < size) {
            return data[2 + current_key_index_];
        }
        return 0;
    }
    
    // 获取当前索引条目的RID
    RID GetRID() const {
        if (is_end_ || current_leaf_page_ == nullptr || current_key_index_ < 0) {
            return RID();
        }
        const int* data = reinterpret_cast<const int*>(current_leaf_page_->GetData());
        int size = data[1];
        if (current_key_index_ < size) {
            RID rid;
            rid.page_id = data[2 + size + current_key_index_ * 2];
            rid.slot_num = data[2 + size + current_key_index_ * 2 + 1];
            return rid;
        }
        return RID();
    }
    
    void SetEnd(bool is_end) {
        is_end_ = is_end;
    }
    
    bool IsEnd() const {
        return is_end_;
    }
    
    int GetCurrentPageId() const {
        return current_page_id_;
    }
    
    int GetCurrentKeyIndex() const {
        return current_key_index_;
    }
    
    Page *GetCurrentPage() const {
        return current_leaf_page_;
    }
    
    void SetCurrentPage(Page *page, int page_id) {
        current_leaf_page_ = page;
        current_page_id_ = page_id;
    }
};

#endif // INDEX_ITERATOR_H

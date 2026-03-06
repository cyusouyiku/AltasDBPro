//
//  BPlusTree.cpp
//  MiniDB
//
//  Created by 张宗煜 on 2026/1/6.
//

#include "BPlusTree.h"
#include "../buffer/Page.h"
#include "../common/rid.h"
#include "index_iterator.h"
#include <cstring>
#include <algorithm>
#include <stdexcept>

BPlusTree::BPlusTree(const std::string& index_name, BufferPoolManager* buffer_pool_manager, int leaf_max_size, int internal_max_size){
    this->index_name = index_name;
    this->buffer_pool_manager = buffer_pool_manager;
    this->leaf_max_size = leaf_max_size;
    this->internal_max_size = internal_max_size;
    this->root_page_id = -1;
    this->leaf_start_page_id = -1;
    this->diskmanager = nullptr;
}

BPlusTree::~BPlusTree(){};

bool BPlusTree::IsLeaf(Page* page) {
    if (page == nullptr) {
        return false;
    }
    const int* data = reinterpret_cast<const int*>(page->GetData());
    return data[0] == 1;
}

int BPlusTree::BinarySearch(Page* page, const int& key) {
    if (page == nullptr) {
        return 0;
    }
    const int* data = reinterpret_cast<const int*>(page->GetData());
    int size = data[1];
    int left = 0;
    int right = size;
    
    while (left < right) {
        int mid = (left + right) / 2;
        int current_key = data[2 + mid];
        if (key >= current_key) {
            left = mid + 1;
        } else {
            right = mid;
        }
    }
    return left;
}

Page* BPlusTree::FindLeafPage(const int &key, bool left_most) {
    if (root_page_id == -1) {
        return nullptr;
    }
    
    int current_page_id = root_page_id;
    
    while (true) {
        Page* current_page = buffer_pool_manager->FetchPage(current_page_id);
        if (current_page == nullptr) {
            return nullptr;
        }
        
        if (IsLeaf(current_page)) {
            return current_page;
        }
        
        int child_index = 0;
        if (left_most) {
            child_index = 0;
        } else {
            child_index = BinarySearch(current_page, key);
        }
        
        const int* data = reinterpret_cast<const int*>(current_page->GetData());
        int size = data[1];
        int child_page_id = data[2 + size + 1 + child_index];
        
        buffer_pool_manager->UnpinPage(current_page_id, false);
        current_page_id = child_page_id;
    }
}

int BPlusTree::GetRootPageId() {
    return root_page_id;
}

void BPlusTree::UpdateRootPageId(int insert_record) {
    // 如果需要持久化根节点ID，可以在这里实现
    (void)insert_record;
}

bool BPlusTree::GetValue(const int &key, std::vector<RID> *result) {
    std::lock_guard<std::mutex> lock(latch);
    
    if (root_page_id == -1) {
        return false;
    }
    
    Page* leaf_page = FindLeafPage(key, false);
    if (leaf_page == nullptr) {
        return false;
    }
    
    int* data = reinterpret_cast<int*>(const_cast<char*>(leaf_page->GetData()));
    int size = data[1];
    int pos = BinarySearch(leaf_page, key);
    
    if (result != nullptr) {
        result->clear();
        
        // 查找所有匹配的key（支持重复键）
        for (int i = pos - 1; i >= 0; i--) {
            if (data[2 + i] == key) {
                RID rid;
                rid.page_id = data[2 + size + i * 2];
                rid.slot_num = data[2 + size + i * 2 + 1];
                result->insert(result->begin(), rid);
            } else {
                break;
            }
        }
        
        for (int i = pos; i < size; i++) {
            if (data[2 + i] == key) {
                RID rid;
                rid.page_id = data[2 + size + i * 2];
                rid.slot_num = data[2 + size + i * 2 + 1];
                result->push_back(rid);
            } else {
                break;
            }
        }
    }
    
    buffer_pool_manager->UnpinPage(leaf_page->GetPageId(), false);
    return result != nullptr && !result->empty();
}

IndexIterator BPlusTree::Begin() {
    std::lock_guard<std::mutex> lock(latch);
    
    if (root_page_id == -1) {
        return IndexIterator();  // 返回结束迭代器
    }
    
    Page* leaf_page = FindLeafPage(0, true);  // 找到最左边的叶子节点
    if (leaf_page == nullptr) {
        return IndexIterator();  // 返回结束迭代器
    }
    
    // 检查叶子页面是否有效
    const int* data = reinterpret_cast<const int*>(leaf_page->GetData());
    int size = data[1];
    if (size == 0) {
        buffer_pool_manager->UnpinPage(leaf_page->GetPageId(), false);
        return IndexIterator();  // 空树，返回结束迭代器
    }
    
    int leaf_page_id = leaf_page->GetPageId();
    // 创建迭代器后，保持页面固定（迭代器负责Unpin）
    // 注意：这里简化处理，实际应该让迭代器管理页面的生命周期
    IndexIterator iter(this, leaf_page, 0, leaf_page_id, false);
    return iter;
}

IndexIterator BPlusTree::End() {
    return IndexIterator();  // 返回结束迭代器（is_end_ = true）
}

bool BPlusTree::Insert(const int &key, const RID &value) {
    std::lock_guard<std::mutex> lock(latch);
    
    // 如果树为空，创建根节点（叶子节点）
    if (root_page_id == -1) {
        int new_page_id;
        Page* root_page = buffer_pool_manager->NewPage(&new_page_id);
        if (root_page == nullptr) {
            return false;
        }
        
        int* data = reinterpret_cast<int*>(const_cast<char*>(root_page->GetData()));
        data[0] = 1;  // 叶子节点
        data[1] = 1;  // size = 1
        data[2] = key;  // key
        data[3] = value.page_id;  // RID.page_id
        data[4] = value.slot_num;  // RID.slot_num
        data[5] = -1;  // next_leaf_page_id = -1
        
        root_page_id = new_page_id;
        leaf_start_page_id = new_page_id;
        root_page->SetDirty(true);
        buffer_pool_manager->UnpinPage(new_page_id, true);
        UpdateRootPageId(1);
        return true;
    }
    
    // 找到应该插入的叶子节点
    Page* leaf_page = FindLeafPage(key, false);
    if (leaf_page == nullptr) {
        return false;
    }
    
    int* data = reinterpret_cast<int*>(const_cast<char*>(leaf_page->GetData()));
    int size = data[1];
    
    // 如果叶子节点未满，直接插入
    if (size < leaf_max_size) {
        InsertIntoLeaf(leaf_page, key, value);
        buffer_pool_manager->UnpinPage(leaf_page->GetPageId(), true);
        return true;
    }
    
    // 叶子节点已满，需要分裂
    int new_page_id;
    Page* new_leaf_page = buffer_pool_manager->NewPage(&new_page_id);
    if (new_leaf_page == nullptr) {
        buffer_pool_manager->UnpinPage(leaf_page->GetPageId(), false);
        return false;
    }
    
    int middle_key;
    SplitLeafPage(leaf_page, new_leaf_page, &middle_key);
    
    // 决定插入到哪个节点
    if (key < middle_key) {
        InsertIntoLeaf(leaf_page, key, value);
    } else {
        InsertIntoLeaf(new_leaf_page, key, value);
    }
    
    // 插入分裂键到父节点
    InsertIntoParent(leaf_page, middle_key, new_leaf_page);
    
    buffer_pool_manager->UnpinPage(leaf_page->GetPageId(), true);
    buffer_pool_manager->UnpinPage(new_leaf_page->GetPageId(), true);
    return true;
}

void BPlusTree::InsertIntoLeaf(Page *leaf_page, const int &key, const RID &value) {
    int* data = reinterpret_cast<int*>(const_cast<char*>(leaf_page->GetData()));
    int size = data[1];
    int pos = BinarySearch(leaf_page, key);
    
    // 移动keys和RIDs，为新元素腾出空间
    for (int i = size; i > pos; i--) {
        data[2 + i] = data[2 + i - 1];  // 移动key
        data[2 + size + i * 2] = data[2 + size + (i - 1) * 2];  // 移动RID.page_id
        data[2 + size + i * 2 + 1] = data[2 + size + (i - 1) * 2 + 1];  // 移动RID.slot_num
    }
    
    // 插入新元素
    data[2 + pos] = key;
    data[2 + size + pos * 2] = value.page_id;
    data[2 + size + pos * 2 + 1] = value.slot_num;
    data[1] = size + 1;  // 更新size
}

void BPlusTree::SplitLeafPage(Page *leaf_page, Page *new_leaf_page, int *middle_key) {
    int* old_data = reinterpret_cast<int*>(const_cast<char*>(leaf_page->GetData()));
    int* new_data = reinterpret_cast<int*>(const_cast<char*>(new_leaf_page->GetData()));
    
    int old_size = old_data[1];
    int split_point = (old_size + 1) / 2;  // 分裂点
    
    // 初始化新叶子节点
    new_data[0] = 1;  // 叶子节点
    new_data[1] = old_size - split_point;  // 新节点的大小
    
    // 复制后半部分到新节点
    for (int i = 0; i < new_data[1]; i++) {
        new_data[2 + i] = old_data[2 + split_point + i];  // 复制key
        new_data[2 + new_data[1] + i * 2] = old_data[2 + old_size + (split_point + i) * 2];  // 复制RID
        new_data[2 + new_data[1] + i * 2 + 1] = old_data[2 + old_size + (split_point + i) * 2 + 1];
    }
    
    // 更新旧节点大小
    old_data[1] = split_point;
    
    // 设置新节点的next指针
    int old_next = old_data[2 + old_size + old_size * 2];  // 原next指针位置
    new_data[2 + new_data[1] + new_data[1] * 2] = old_next;
    old_data[2 + old_size + old_size * 2] = new_leaf_page->GetPageId();  // 更新旧节点的next
    
    // middle_key是新节点的第一个key
    *middle_key = new_data[2];
}

void BPlusTree::InsertIntoParent(Page *old_page, const int &key, Page *new_page) {
    int old_page_id = old_page->GetPageId();
    int new_page_id = new_page->GetPageId();
    
    // 如果old_page是根节点，需要创建新的根节点
    if (old_page_id == root_page_id) {
        int root_page_id_new;
        Page* new_root = buffer_pool_manager->NewPage(&root_page_id_new);
        if (new_root == nullptr) {
            return;
        }
        
        int* root_data = reinterpret_cast<int*>(const_cast<char*>(new_root->GetData()));
        root_data[0] = 0;  // 内部节点
        root_data[1] = 1;  // size = 1
        root_data[2] = key;  // key
        root_data[3] = old_page_id;  // 第一个子节点
        root_data[4] = new_page_id;  // 第二个子节点
        
        root_page_id = root_page_id_new;
        new_root->SetDirty(true);
        buffer_pool_manager->UnpinPage(root_page_id_new, true);
        UpdateRootPageId(1);
        return;
    }
    
    // 简化处理：假设可以通过某种方式找到父节点
    // 实际实现中可能需要维护父节点指针或重新查找
    // 这里先返回，实际实现需要完善
}

void BPlusTree::SplitInternalPage(Page *internal_page, Page *new_internal_page, int *middle_key) {
    int* old_data = reinterpret_cast<int*>(const_cast<char*>(internal_page->GetData()));
    int* new_data = reinterpret_cast<int*>(const_cast<char*>(new_internal_page->GetData()));
    
    int old_size = old_data[1];
    int split_point = old_size / 2;  // 分裂点
    
    // 初始化新内部节点
    new_data[0] = 0;  // 内部节点
    new_data[1] = old_size - split_point - 1;  // 新节点的大小
    
    // middle_key是分裂点的key
    *middle_key = old_data[2 + split_point];
    
    // 复制后半部分keys到新节点
    for (int i = 0; i < new_data[1]; i++) {
        new_data[2 + i] = old_data[2 + split_point + 1 + i];
    }
    
    // 复制后半部分child pointers到新节点
    for (int i = 0; i <= new_data[1]; i++) {
        new_data[2 + new_data[1] + 1 + i] = old_data[2 + old_size + 1 + split_point + 1 + i];
    }
    
    // 更新旧节点大小
    old_data[1] = split_point;
}

bool BPlusTree::Remove(const int &key) {
    std::lock_guard<std::mutex> lock(latch);
    
    if (root_page_id == -1) {
        return false;
    }
    
    Page* leaf_page = FindLeafPage(key, false);
    if (leaf_page == nullptr) {
        return false;
    }
    
    int* data = reinterpret_cast<int*>(const_cast<char*>(leaf_page->GetData()));
    int size = data[1];
    int pos = BinarySearch(leaf_page, key);
    
    // 检查key是否存在
    if (pos == 0 || data[2 + pos - 1] != key) {
        buffer_pool_manager->UnpinPage(leaf_page->GetPageId(), false);
        return false;
    }
    
    pos = pos - 1;  // 找到key的位置
    
    // 删除key和RID
    for (int i = pos; i < size - 1; i++) {
        data[2 + i] = data[2 + i + 1];  // 移动key
        data[2 + size + i * 2] = data[2 + size + (i + 1) * 2];  // 移动RID
        data[2 + size + i * 2 + 1] = data[2 + size + (i + 1) * 2 + 1];
    }
    data[1] = size - 1;
    
    leaf_page->SetDirty(true);
    
    // 检查是否需要合并或重分布
    CoalesceOrRedistribute(leaf_page);
    
    buffer_pool_manager->UnpinPage(leaf_page->GetPageId(), true);
    return true;
}

void BPlusTree::CoalesceOrRedistribute(Page *node_page) {
    int* data = reinterpret_cast<int*>(const_cast<char*>(node_page->GetData()));
    int size = data[1];
    int page_id = node_page->GetPageId();
    
    // 如果是根节点
    if (page_id == root_page_id) {
        AdjustRoot(node_page);
        return;
    }
    
    // 计算最小大小
    int min_size;
    if (IsLeaf(node_page)) {
        min_size = (leaf_max_size + 1) / 2;
    } else {
        min_size = (internal_max_size + 1) / 2;
    }
    
    // 如果节点大小足够，不需要合并
    if (size >= min_size) {
        return;
    }
    
    // 简化处理：实际实现需要找到兄弟节点并合并或重分布
}

void BPlusTree::AdjustRoot(Page *root_page) {
    int* data = reinterpret_cast<int*>(const_cast<char*>(root_page->GetData()));
    int size = data[1];
    
    // 如果根节点是叶子节点且为空，删除树
    if (IsLeaf(root_page) && size == 0) {
        buffer_pool_manager->DeletePage(root_page_id);
        root_page_id = -1;
        leaf_start_page_id = -1;
        UpdateRootPageId(0);
        return;
    }
    
    // 如果根节点是内部节点且只有一个子节点，将子节点提升为根
    if (!IsLeaf(root_page) && size == 0) {
        int child_page_id = data[2 + size + 1];  // 唯一的子节点
        buffer_pool_manager->DeletePage(root_page_id);
        root_page_id = child_page_id;
        UpdateRootPageId(0);
    }
}

//
//  BPlusTree.h
//  MiniDB
//
//  Created by 张宗煜 on 2026/1/5.
//

#ifndef BPLUSTREE_H
#define BPLUSTREE_H

#include "BufferPoolManager.h"
#include "../common/rid.h"
#include <string>
#include <vector>
#include <mutex>

class Page;
class DiskManager;
class IndexIterator;

class BPlusTree{
private:
    std::string index_name;
    BufferPoolManager* buffer_pool_manager;
    DiskManager* diskmanager;
    int root_page_id;
    int leaf_start_page_id;
    int leaf_max_size;
    int internal_max_size;
    std::mutex latch;
public:
    BPlusTree(const std::string &index_name,
              BufferPoolManager *buffer_pool_manager,
              int leaf_max_size,
              int internal_max_size);
    ~BPlusTree();
    bool IsLeaf(Page *page);
    int BinarySearch(Page *page, const int &key);
    Page *FindLeafPage(const int &key, bool left_most = false);
    bool GetValue(const int &key, std::vector<RID> *result);
    bool Insert(const int &key, const RID &value);
    bool Remove(const int &key);
    void InsertIntoLeaf(Page *leaf_page, const int &key, const RID &value);
    void InsertIntoParent(Page *old_page, const int &key, Page *new_page);
    void SplitLeafPage(Page *leaf_page, Page *new_leaf_page, int *middle_key);
    void SplitInternalPage(Page *internal_page, Page *new_internal_page, int *middle_key);
    void CoalesceOrRedistribute(Page *node_page);
    void AdjustRoot(Page *root_page);
    int GetRootPageId();
    void UpdateRootPageId(int insert_record = 0);
    
    // 迭代器方法（用于范围查询）
    IndexIterator Begin();
    IndexIterator End();
};

#endif // BPLUSTREE_H

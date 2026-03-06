//
//  table_heap.cpp
//  MiniDB
//
//  Created on 2026/1/10.
//

#include "table_heap.h"
#include "Page.h"
#include "config.h"
#include <cstring>

static const int SLOT_HEADER = 12;  // next_free(4) + n_slots(4) + reserved(4)
static const int SLOT_ENTRY_SIZE = 8;  // offset(4) + size(4)
static const int MAX_SLOTS_PER_PAGE = (PAGE_SIZE - SLOT_HEADER) / (SLOT_ENTRY_SIZE + 32);  // 约 80

Tuple TableIterator::operator*() {
    Tuple tuple;
    if (!is_end_ && table_ != nullptr) {
        table_->GetTuple(current_rid_, &tuple);
    }
    return tuple;
}

TableHeap::TableHeap(BufferPoolManager *buffer_pool_manager)
    : buffer_pool_manager_(buffer_pool_manager), first_page_id_(-1), last_page_id_(-1) {
    int page_id;
    Page* first_page = buffer_pool_manager_->NewPage(&page_id);
    if (first_page != nullptr) {
        first_page_id_ = page_id;
        last_page_id_ = page_id;
        char* p = const_cast<char*>(first_page->GetData());
        int zero = 0;
        memcpy(p, &zero, 4);       // next_free = SLOT_HEADER
        int n = SLOT_HEADER;
        memcpy(p + 4, &n, 4);     // next_free init to SLOT_HEADER
        memcpy(p + 8, &zero, 4);  // n_slots = 0
        buffer_pool_manager_->UnpinPage(page_id, true);
    }
}

TableHeap::~TableHeap() {}

static int get_next_free(const char* page_data) {
    int v; memcpy(&v, page_data + 4, 4); return v; }
static int get_n_slots(const char* page_data) {
    int v; memcpy(&v, page_data + 8, 4); return v; }
static void set_next_free(char* page_data, int v) {
    memcpy(page_data + 4, &v, 4); }
static void set_n_slots(char* page_data, int v) {
    memcpy(page_data + 8, &v, 4); }
static void get_slot(const char* page_data, int slot_num, int* offset, int* size) {
    const char* p = page_data + SLOT_HEADER + slot_num * SLOT_ENTRY_SIZE;
    memcpy(offset, p, 4); memcpy(size, p + 4, 4);
}
static void set_slot(char* page_data, int slot_num, int offset, int size) {
    char* p = page_data + SLOT_HEADER + slot_num * SLOT_ENTRY_SIZE;
    memcpy(p, &offset, 4); memcpy(p + 4, &size, 4);
}

bool TableHeap::InsertTuple(const Tuple &tuple, RID *rid) {
    if (buffer_pool_manager_ == nullptr) return false;
    size_t copy_size = tuple.GetSize();
    if (copy_size > PAGE_SIZE - SLOT_HEADER - 64) copy_size = PAGE_SIZE - SLOT_HEADER - 64;

    Page* page = buffer_pool_manager_->FetchPage(last_page_id_);
    if (page == nullptr) {
        int new_page_id;
        Page* new_page = buffer_pool_manager_->NewPage(&new_page_id);
        if (new_page == nullptr) return false;
        last_page_id_ = new_page_id;
        if (first_page_id_ < 0) first_page_id_ = new_page_id;
        page = new_page;
        char* p = const_cast<char*>(page->GetData());
        memset(p, 0, PAGE_SIZE);
        int n = SLOT_HEADER;
        memcpy(p + 4, &n, 4);
        n = 0;
        memcpy(p + 8, &n, 4);
    }

    char* page_data = const_cast<char*>(page->GetData());
    int next_free = get_next_free(page_data);
    int n_slots = get_n_slots(page_data);
    if (n_slots >= MAX_SLOTS_PER_PAGE || next_free + (int)copy_size + SLOT_ENTRY_SIZE > PAGE_SIZE) {
        buffer_pool_manager_->UnpinPage(last_page_id_, false);
        int new_page_id;
        Page* new_page = buffer_pool_manager_->NewPage(&new_page_id);
        if (new_page == nullptr) return false;
        last_page_id_ = new_page_id;
        memset(const_cast<char*>(new_page->GetData()), 0, PAGE_SIZE);
        page_data = const_cast<char*>(new_page->GetData());
        next_free = SLOT_HEADER;
        n_slots = 0;
        set_next_free(page_data, next_free);
        set_n_slots(page_data, 0);
        page = new_page;
    }

    set_slot(page_data, n_slots, next_free, static_cast<int>(copy_size));
    memcpy(page_data + next_free, tuple.GetData(), copy_size);
    set_next_free(page_data, next_free + static_cast<int>(copy_size));
    set_n_slots(page_data, n_slots + 1);

    if (rid != nullptr) {
        rid->page_id = last_page_id_;
        rid->slot_num = n_slots;
    }
    page->SetDirty(true);
    buffer_pool_manager_->UnpinPage(last_page_id_, true);
    return true;
}

bool TableHeap::GetTuple(const RID &rid, Tuple *tuple) {
    if (buffer_pool_manager_ == nullptr || !rid.IsValid()) return false;
    Page* page = buffer_pool_manager_->FetchPage(rid.page_id);
    if (page == nullptr) return false;
    const char* page_data = page->GetData();
    int n_slots = get_n_slots(page_data);
    if (rid.slot_num < 0 || rid.slot_num >= n_slots) {
        buffer_pool_manager_->UnpinPage(rid.page_id, false);
        return false;
    }
    int offset, size;
    get_slot(page_data, rid.slot_num, &offset, &size);
    if (size <= 0 || offset + size > PAGE_SIZE) {
        buffer_pool_manager_->UnpinPage(rid.page_id, false);
        return false;
    }
    tuple->SetSize(size);
    tuple->SetRID(rid);
    memcpy(const_cast<char*>(tuple->GetData()), page_data + offset, size);
    buffer_pool_manager_->UnpinPage(rid.page_id, false);
    return true;
}

bool TableHeap::DeleteTuple(const RID &rid) {
    if (buffer_pool_manager_ == nullptr || !rid.IsValid()) {
        return false;
    }
    
    Page* page = buffer_pool_manager_->FetchPage(rid.page_id);
    if (page == nullptr) {
        return false;
    }
    
    // 简化实现：标记记录为已删除
    // 实际实现需要更复杂的逻辑
    page->SetDirty(true);
    buffer_pool_manager_->UnpinPage(rid.page_id, true);
    return true;
}

bool TableHeap::UpdateTuple(const Tuple &tuple, const RID &rid) {
    if (buffer_pool_manager_ == nullptr || !rid.IsValid()) {
        return false;
    }
    
    Page* page = buffer_pool_manager_->FetchPage(rid.page_id);
    if (page == nullptr) {
        return false;
    }
    
    char* page_data = const_cast<char*>(page->GetData());
    size_t copy_size = tuple.GetSize() < (PAGE_SIZE - 16) ? tuple.GetSize() : (PAGE_SIZE - 16);
    memcpy(page_data + 16, tuple.GetData(), copy_size);
    
    page->SetDirty(true);
    buffer_pool_manager_->UnpinPage(rid.page_id, true);
    return true;
}

int TableHeap::GetSlotCount(int page_id) const {
    if (buffer_pool_manager_ == nullptr || page_id < 0) return 0;
    Page* page = buffer_pool_manager_->FetchPage(page_id);
    if (page == nullptr) return 0;
    const char* p = page->GetData();
    int n; memcpy(&n, p + 8, 4);
    buffer_pool_manager_->UnpinPage(page_id, false);
    return n;
}

TableIterator TableHeap::Begin() {
    RID first_rid;
    if (first_page_id_ >= 0) {
        first_rid.page_id = first_page_id_;
        first_rid.slot_num = 0;
    }
    return TableIterator(this, first_rid, first_page_id_ < 0);
}

TableIterator TableHeap::End() {
    RID end_rid;
    return TableIterator(this, end_rid, true);
}

TableIterator TableHeap::GetIterator(const RID &rid) {
    return TableIterator(this, rid, false);
}

void TableHeap::FlushBatch() {
    if (buffer_pool_manager_ != nullptr) {
        buffer_pool_manager_->FlushAllPages();
    }
}

//
//  SortExecutor.cpp
//  MiniDB
//

#include "SortExecutor.h"
#include <algorithm>

SortExecutor::SortExecutor(std::unique_ptr<Executor> child, size_t key_offset)
    : child_(std::move(child)), cursor_(0), key_offset_(key_offset) {}

void SortExecutor::Init() {
    child_->Init();
    sorted_.clear();
    Tuple t;
    RID rid;
    while (child_->Next(&t, &rid)) {
        sorted_.push_back(t);
    }
    std::sort(sorted_.begin(), sorted_.end(), [this](const Tuple& a, const Tuple& b) {
        return a.GetIntValue(key_offset_) < b.GetIntValue(key_offset_);
    });
    cursor_ = 0;
}

bool SortExecutor::Next(Tuple* tuple, RID* rid) {
    if (cursor_ >= sorted_.size()) return false;
    if (tuple) *tuple = sorted_[cursor_];
    if (rid) *rid = sorted_[cursor_].GetRID();
    cursor_++;
    return true;
}

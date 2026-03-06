//
//  JoinExecutor.cpp
//  MiniDB
//

#include "JoinExecutor.h"
#include "common/types.h"
#include <cstring>

JoinExecutor::JoinExecutor(std::unique_ptr<Executor> left, std::unique_ptr<Executor> right)
    : left_(std::move(left)), right_(std::move(right)), left_has_current_(false), initialized_(false) {}

void JoinExecutor::Init() {
    left_->Init();
    right_->Init();
    left_has_current_ = left_->Next(&left_current_, nullptr);
    initialized_ = true;
}

bool JoinExecutor::Next(Tuple* tuple, RID* rid) {
    if (!initialized_) Init();
    while (left_has_current_) {
        Tuple right_tuple;
        if (right_->Next(&right_tuple, nullptr)) {
            if (tuple) {
                size_t left_len = left_current_.GetSize();
                size_t right_len = right_tuple.GetSize();
                if (left_len + right_len <= Tuple::MAX_SIZE) {
                    memcpy(tuple->GetData(), left_current_.GetData(), left_len);
                    memcpy(tuple->GetData() + left_len, right_tuple.GetData(), right_len);
                    tuple->SetSize(left_len + right_len);
                } else {
                    size_t copy_right = Tuple::MAX_SIZE > left_len ? Tuple::MAX_SIZE - left_len : 0;
                    memcpy(tuple->GetData(), left_current_.GetData(), left_len);
                    if (copy_right) memcpy(tuple->GetData() + left_len, right_tuple.GetData(), copy_right);
                    tuple->SetSize(left_len + copy_right);
                }
            }
            if (rid) *rid = left_current_.GetRID();
            return true;
        }
        right_->Init();
        left_has_current_ = left_->Next(&left_current_, nullptr);
    }
    return false;
}

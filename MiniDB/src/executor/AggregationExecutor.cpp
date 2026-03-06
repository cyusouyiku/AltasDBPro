//
//  AggregationExecutor.cpp
//  MiniDB
//

#include "AggregationExecutor.h"
#include <cstring>

AggregationExecutor::AggregationExecutor(std::unique_ptr<Executor> child, AggregationType agg_type, size_t key_offset)
    : child_(std::move(child)), agg_type_(agg_type), key_offset_(key_offset),
      done_(false), initialized_(false), count_(0), sum_(0), min_val_(0), max_val_(0), has_value_(false) {}

void AggregationExecutor::Init() {
    child_->Init();
    done_ = false;
    initialized_ = true;
    count_ = 0;
    sum_ = 0;
    has_value_ = false;
    Tuple t;
    while (child_->Next(&t, nullptr)) {
        count_++;
        int v = t.GetIntValue(key_offset_);
        sum_ += v;
        if (!has_value_) {
            min_val_ = max_val_ = v;
            has_value_ = true;
        } else {
            if (v < min_val_) min_val_ = v;
            if (v > max_val_) max_val_ = v;
        }
    }
}

bool AggregationExecutor::Next(Tuple* tuple, RID* rid) {
    if (!initialized_) Init();
    if (done_) return false;
    done_ = true;
    if (!tuple) return true;
    tuple->SetSize(sizeof(int));
    int result = 0;
    switch (agg_type_) {
        case AggregationType::COUNT: result = static_cast<int>(count_); break;
        case AggregationType::SUM:  result = static_cast<int>(sum_); break;
        case AggregationType::AVG:  result = count_ ? static_cast<int>(sum_ / count_) : 0; break;
        case AggregationType::MIN:  result = has_value_ ? min_val_ : 0; break;
        case AggregationType::MAX:  result = has_value_ ? max_val_ : 0; break;
    }
    memcpy(tuple->GetData(), &result, sizeof(int));
    if (rid) rid->page_id = -1, rid->slot_num = -1;
    return true;
}

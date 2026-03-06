//
//  LayeredClock.cpp
//  MiniDB - 分层优先级替换策略实现
//

#include "LayeredClock.h"
#include <mutex>

LayeredClock::LayeredClock(size_t frame_count)
    : frame_count_(frame_count), clock_hand_(0) {
    use_bits_.resize(frame_count, false);
    in_replacer_.resize(frame_count, false);
    is_hot_.resize(frame_count, true);  // 默认热页
}

LayeredClock::~LayeredClock() = default;

bool LayeredClock::Victim(int& frame_id) {
    std::lock_guard<std::mutex> lock(latch_);
    if (Size() == 0) {
        return false;
    }

    // 第一轮：优先淘汰冷页（冷页淘汰优先级为热页的5倍）
    int start_hand = clock_hand_;
    int cold_scan_count = 0;
    do {
        if (in_replacer_[clock_hand_]) {
            if (!use_bits_[clock_hand_] && !is_hot_[clock_hand_]) {
                frame_id = static_cast<int>(clock_hand_);
                in_replacer_[clock_hand_] = false;
                clock_hand_ = (clock_hand_ + 1) % static_cast<int>(frame_count_);
                return true;
            }
            if (!use_bits_[clock_hand_] && is_hot_[clock_hand_]) {
                cold_scan_count++;
                if (cold_scan_count >= COLD_EVICTION_PRIORITY) {
                    frame_id = static_cast<int>(clock_hand_);
                    in_replacer_[clock_hand_] = false;
                    clock_hand_ = (clock_hand_ + 1) % static_cast<int>(frame_count_);
                    return true;
                }
            }
            if (use_bits_[clock_hand_]) {
                use_bits_[clock_hand_] = false;
            }
        }
        clock_hand_ = (clock_hand_ + 1) % static_cast<int>(frame_count_);
    } while (clock_hand_ != start_hand);

    // 第二轮：无冷页可淘汰时，淘汰热页
    start_hand = clock_hand_;
    do {
        if (in_replacer_[clock_hand_] && !use_bits_[clock_hand_]) {
            frame_id = static_cast<int>(clock_hand_);
            in_replacer_[clock_hand_] = false;
            clock_hand_ = (clock_hand_ + 1) % static_cast<int>(frame_count_);
            return true;
        }
        if (use_bits_[clock_hand_]) {
            use_bits_[clock_hand_] = false;
        }
        clock_hand_ = (clock_hand_ + 1) % static_cast<int>(frame_count_);
    } while (clock_hand_ != start_hand);

    return false;
}

void LayeredClock::Pin(int frame_id) {
    std::lock_guard<std::mutex> lock(latch_);
    if (frame_id >= 0 && frame_id < static_cast<int>(frame_count_)) {
        in_replacer_[frame_id] = false;
    }
}

void LayeredClock::Unpin(int frame_id) {
    Unpin(frame_id, true);
}

void LayeredClock::Unpin(int frame_id, bool is_hot) {
    std::lock_guard<std::mutex> lock(latch_);
    if (frame_id >= 0 && frame_id < static_cast<int>(frame_count_)) {
        if (!in_replacer_[frame_id]) {
            in_replacer_[frame_id] = true;
            use_bits_[frame_id] = true;
            is_hot_[frame_id] = is_hot;
        }
    }
}

size_t LayeredClock::Size() {
    std::lock_guard<std::mutex> lock(latch_);
    size_t count = 0;
    for (size_t i = 0; i < frame_count_; ++i) {
        if (in_replacer_[i]) count++;
    }
    return count;
}

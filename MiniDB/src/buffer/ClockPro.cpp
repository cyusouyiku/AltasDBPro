#include "ClockPro.h"
#include <algorithm>

ClockPro::ClockPro(size_t frame)
    : frame_(frame), clock_hand_(0) {
    u_bits_.resize(frame_, false);
    p_bits_.resize(frame_, false);
    in_replacer_.resize(frame_, false);
}

ClockPro::~ClockPro() = default;

bool ClockPro::Victim(int& frame_id) {
    std::lock_guard<std::mutex> lock(latch_);
    if (Size() == 0) {
        return false;
    }
    int start_hand = clock_hand_;
    do {
        if (in_replacer_[clock_hand_]) {
            if (!u_bits_[clock_hand_]) {
                frame_id = static_cast<int>(clock_hand_);
                in_replacer_[clock_hand_] = false;
                if (p_bits_[clock_hand_]) {
                    p_bits_[clock_hand_] = false;  // 新页变老页
                }
                clock_hand_ = (clock_hand_ + 1) % static_cast<int>(frame_);
                return true;
            } else {
                u_bits_[clock_hand_] = false;
            }
        }
        clock_hand_ = (clock_hand_ + 1) % static_cast<int>(frame_);
    } while (clock_hand_ != start_hand);
    return false;
}

void ClockPro::Pin(int frame_id) {
    std::lock_guard<std::mutex> lock(latch_);
    if (frame_id >= 0 && frame_id < static_cast<int>(frame_)) {
        in_replacer_[frame_id] = false;
    }
}

void ClockPro::Unpin(int frame_id) {
    std::lock_guard<std::mutex> lock(latch_);
    if (frame_id >= 0 && frame_id < static_cast<int>(frame_)) {
        if (!in_replacer_[frame_id]) {
            in_replacer_[frame_id] = true;
            u_bits_[frame_id] = true;
            p_bits_[frame_id] = true;  // 新加入的页
        }
    }
}

size_t ClockPro::Size() {
    std::lock_guard<std::mutex> lock(latch_);
    size_t count = 0;
    for (size_t i = 0; i < frame_; ++i) {
        if (in_replacer_[i]) count++;
    }
    return count;
}

bool ClockPro::IsOld(int frame_id) const {
    return frame_id >= 0 && frame_id < static_cast<int>(frame_) && !p_bits_[frame_id];
}

//
//  Clock.cpp
//  MiniDB
//
//  Created by 张宗煜 on 2026/1/4.
//

#include "Clock.h"
#include <mutex>

Clock::Clock(size_t frame)
    : frame(frame), clock_hand(0) {
    use_bits.resize(frame, false);
    in_replacer.resize(frame, false);
}

Clock::~Clock() {
}

bool Clock::Victim(int& frame_id) {
    std::lock_guard<std::mutex> lock(latch);
    if (Size() == 0) {
        return false;
    }
    
    // 从 clock_hand 开始循环查找可淘汰的帧
    int start_hand = clock_hand;
    do {
        if (in_replacer[clock_hand]) {
            // 如果 use_bit 为 0，可以淘汰
            if (!use_bits[clock_hand]) {
                frame_id = static_cast<int>(clock_hand);
                in_replacer[clock_hand] = false;
                clock_hand = (clock_hand + 1) % static_cast<int>(frame);
                return true;
            } else {
                // use_bit 为 1，将其设为 0，继续查找
                use_bits[clock_hand] = false;
            }
        }
        // 移动时钟指针
        clock_hand = (clock_hand + 1) % static_cast<int>(frame);
    } while (clock_hand != start_hand);
    
    // 遍历一圈都没找到，返回 false
    return false;
}

void Clock::Pin(int frame_id) {
    std::lock_guard<std::mutex> lock(latch);
    if (frame_id < 0 || frame_id >= static_cast<int>(frame)) {
        return;
    }
        if (in_replacer[frame_id]) {
        in_replacer[frame_id] = false;
    }
}

void Clock::Unpin(int frame_id) {
    Unpin(frame_id, true);
}

void Clock::Unpin(int frame_id, bool /* is_hot */) {
    std::lock_guard<std::mutex> lock(latch);
    if (frame_id < 0 || frame_id >= static_cast<int>(frame)) {
        return;
    }
    if (!in_replacer[frame_id]) {
        in_replacer[frame_id] = true;
        use_bits[frame_id] = true;
    }
}

size_t Clock::Size() {
    std::lock_guard<std::mutex> lock(latch);
    size_t count = 0;
    for (size_t i = 0; i < frame; ++i) {
        if (in_replacer[i]) {
            count++;
        }
    }
    return count;
}

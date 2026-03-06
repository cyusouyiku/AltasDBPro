//
//  Clock.h
//  MiniDB
//
//  Created by 张宗煜 on 2026/1/4.
//

#ifndef CLOCK_HPP
#define CLOCK_HPP

#include <vector>
#include <mutex>
#include <cstddef>

class Clock {
private:
    size_t frame;//物理帧总数，也就是内存中可以用的物理页数的上限。
    std::vector<bool> use_bits;//使用位，就是最近有没有被使用过
    std::vector<bool> in_replacer;//表示某个物理帧是否在Clock中
    int clock_hand;//时钟指针，记录当前扫描的物理帧的位置
    std::mutex latch;

public:
    explicit Clock(size_t frame);
    ~Clock();
    bool Victim(int& frame_id);
    void Pin(int frame_id);
    void Unpin(int frame_id);
    void Unpin(int frame_id, bool is_hot);  // 兼容 LayeredClock 接口，Clock 忽略温度
    size_t Size();
};

#endif

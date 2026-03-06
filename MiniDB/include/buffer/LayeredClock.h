//
//  LayeredClock.h
//  MiniDB - 分层优先级替换策略
//  冷页淘汰优先级为热页的5倍，让热数据更长时间驻留内存
//

#ifndef LAYEREDCLOCK_H
#define LAYEREDCLOCK_H

#include <vector>
#include <mutex>
#include <cstddef>

class LayeredClock {
private:
    size_t frame_count_;
    std::vector<bool> use_bits_;      // 近期是否被访问
    std::vector<bool> in_replacer_;    // 是否在替换器中
    std::vector<bool> is_hot_;         // 是否为热页（热页淘汰优先级低）
    int clock_hand_;
    std::mutex latch_;
    static constexpr int COLD_EVICTION_PRIORITY = 5;  // 冷页优先淘汰倍数

public:
    explicit LayeredClock(size_t frame_count);
    ~LayeredClock();

    bool Victim(int& frame_id);
    void Pin(int frame_id);
    void Unpin(int frame_id, bool is_hot = true);  // 支持传入页温度，热页更不易被淘汰
    size_t Size();
};

#endif // LAYEREDCLOCK_H

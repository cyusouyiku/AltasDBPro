#ifndef CLOCKPRO_H
#define CLOCKPRO_H

#include <vector>
#include <mutex>
#include <cstddef>

class ClockPro {
private:
    size_t frame_;
    std::vector<bool> u_bits_;   // 近期是否被访问
    std::vector<bool> p_bits_;    // 新页(true)还是老页(false)
    std::vector<bool> in_replacer_;
    int clock_hand_;
    std::mutex latch_;

public:
    explicit ClockPro(size_t frame);
    ~ClockPro();
    bool Victim(int& frame_id);
    void Pin(int frame_id);
    void Unpin(int frame_id);
    size_t Size();
    bool IsOld(int frame_id) const;
};

#endif
#ifndef PAGE_HPP
#define PAGE_HPP

#include <mutex>
#include <string>
#include <chrono>
#include <cstring>
#include "../common/config.h"

enum class PageTemperature {
    HOT,
    COLD,
    NORMAL
};

class Page {
private:
    char data[PAGE_SIZE];
    int page_id;
    bool is_dirty;
    int pin_count;
    std::mutex latch;
    PageTemperature temperature;
    std::chrono::steady_clock::time_point last_access_ts;
    int access_count;

public:
    Page();
    ~Page();

    void Reset();

    const char *GetData() const;
    char *GetData();

    int GetPageId() const;
    void SetPageId(int page_id);

    int GetPinCount() const;
    void Pin();
    void Unpin();

    bool IsDirty() const;
    void SetDirty(bool is_dirty);

    void Lock();
    void Unlock();

    PageTemperature GetTemperature() const;
    void SetTemperature(PageTemperature temp);

    void UpdateAccessStats();
    void ResetAccessStats();
    int GetAccessCount() const;
    std::chrono::steady_clock::time_point GetLastAccessTime() const;
};

#endif
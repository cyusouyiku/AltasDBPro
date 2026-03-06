#include "Page.h"

Page::Page() : page_id(-1), is_dirty(false), pin_count(0), 
               temperature(PageTemperature::NORMAL), access_count(0) {
    Reset();
    last_access_ts = std::chrono::steady_clock::now();
}

Page::~Page() = default;

void Page::Reset() {
    memset(data, 0, PAGE_SIZE);
    is_dirty = false;
    pin_count = 0;
}

const char *Page::GetData() const {
    return data;
}

char *Page::GetData() {
    return data;
}

int Page::GetPageId() const {
    return page_id;
}

void Page::SetPageId(int page_id) {
    this->page_id = page_id;
}

int Page::GetPinCount() const {
    return pin_count;
}

void Page::Pin() {
    pin_count++;
}

void Page::Unpin() {
    if (pin_count > 0) {
        pin_count--;
    }
}

bool Page::IsDirty() const {
    return is_dirty;
}

void Page::SetDirty(bool is_dirty) {
    this->is_dirty = is_dirty;
}

void Page::Lock() {
    latch.lock();
}

void Page::Unlock() {
    latch.unlock();
}

PageTemperature Page::GetTemperature() const {
    return temperature;
}

void Page::SetTemperature(PageTemperature temp) {
    temperature = temp;
}

void Page::UpdateAccessStats() {
    std::lock_guard<std::mutex> guard(latch);
    access_count++;
    last_access_ts = std::chrono::steady_clock::now();
}

void Page::ResetAccessStats() {
    std::lock_guard<std::mutex> guard(latch);
    access_count = 0;
}

int Page::GetAccessCount() const {
    return access_count;
}

std::chrono::steady_clock::time_point Page::GetLastAccessTime() const {
    return last_access_ts;
}
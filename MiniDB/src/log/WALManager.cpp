//
//  WALManager.cpp
//  MiniDB
//

#include "WALManager.h"
#include <iostream>

WALManager::WALManager(const std::string& log_path) : log_path_(log_path) {
    log_file_.open(log_path_, std::ios::out | std::ios::app | std::ios::binary);
    if (!log_file_.is_open()) {
        log_file_.open(log_path_, std::ios::out | std::ios::binary);
    }
}

WALManager::~WALManager() {
    FlushLogs();
    if (log_file_.is_open()) log_file_.close();
}

bool WALManager::WriteLog(const std::string& log_entry) {
    std::lock_guard<std::mutex> lock(mtx_);
    if (!log_file_.is_open()) return false;
    log_file_.write(log_entry.data(), static_cast<std::streamsize>(log_entry.size()));
    log_file_.put('\n');
    return true;
}

void WALManager::FlushLogs() {
    std::lock_guard<std::mutex> lock(mtx_);
    if (log_file_.is_open()) log_file_.flush();
}
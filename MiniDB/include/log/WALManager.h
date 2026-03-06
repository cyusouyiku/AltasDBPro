//
//  WALManager.h
//  MiniDB
//
//  Created by Framework Generator on 2026/1/24.
//

#ifndef WALMANAGER_H
#define WALMANAGER_H

#include <string>
#include <fstream>
#include <mutex>

class WALManager {
private:
    std::string log_path_;
    std::ofstream log_file_;
    std::mutex mtx_;

public:
    explicit WALManager(const std::string& log_path = "minidb.log");
    ~WALManager();

    bool WriteLog(const std::string& log_entry);
    void FlushLogs();
};

#endif
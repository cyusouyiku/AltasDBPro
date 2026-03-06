//
//  DiskManager.h
//  MiniDB
//
//  Created by 张宗煜 on 2026/1/3.
//

//可以优化也，冷的方固态硬盘热的放内存
#ifndef DISKMANAGER_H
#define DISKMANAGER_H

#include <string>
#include <fstream>
#include <mutex>

class DiskManager {
private:
    std::string db_file_path;
    std::fstream db_file_fd;
    int page_id;
    int next_page_id;
    int file_size;
    std::mutex latch;

public:
    explicit DiskManager(const std::string& db_file_path);
    DiskManager(std::string db_file_path, std::fstream db_file_fd, int page_id, int next_page_id, int file_size);
    ~DiskManager();
    void OpenManager();
    void ReadPage(int page_id, char *page_data);
    void WritePage(int page_id, const char* external_buffer, size_t buffer_size) ;
    int AllocPage();
    void FlushPage();//将指定页写回磁盘（BufferPool调用）
    void FlushAllPages();//将文件中所有页写回磁盘（BufferPool调用）
};

#endif

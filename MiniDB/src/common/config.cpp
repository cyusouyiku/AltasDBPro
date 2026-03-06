//
//  config.cpp
//  MiniDB
//
//  Created by Auto-generated on 2026/1/24.
//

#include "config.h"
#include <cstddef>
#include <string>

// 配置常量定义
namespace config {
    const size_t PAGE_SIZE = 4096;           // 页面大小
    const size_t BUFFER_POOL_SIZE = 100;     // 缓冲池大小
    const int MAX_TRANSACTION_ID = 10000;   // 最大事务ID
    const std::string DB_FILE_NAME = "minidb.db"; // 数据库文件名
}
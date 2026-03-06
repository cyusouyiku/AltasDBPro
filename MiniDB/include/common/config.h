//
//  config.h
//  MiniDB
//
//  Created by 张宗煜 on 2026/1/3.
//
#ifndef CONFIG_H
#define CONFIG_H

constexpr int PAGE_SIZE = 4096;

// 冷热分层配置
constexpr int HOT_THRESHOLD_DAYS = 3;       // 热数据时间阈值（天）
constexpr int BATCH_INSERT_SIZE = 1000;     // 热数据批量写入阈值（行/批）
constexpr int MIGRATION_RATE_LIMIT = 1000;   // 迁移限流（行/秒）
constexpr int HOT_TOP_N_PAGES = 100;         // 热点预加载 Top N 页
constexpr int HOT_ACCESS_WINDOW_HOURS = 1;   // 热点识别时间窗口（小时）

#endif

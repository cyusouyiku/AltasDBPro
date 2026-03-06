//
//  BenchmarkRunner.h
//  MiniDB
//
//  Created by Framework Generator on 2026/1/24.
//

#ifndef BENCHMARKRUNNER_H
#define BENCHMARKRUNNER_H

#include <string>

// 基准测试运行器（规划中）
class BenchmarkRunner {
public:
    BenchmarkRunner();
    ~BenchmarkRunner();

    void RunTPCCTest();
    void RunPerformanceTest();
    void GenerateReport(const std::string& report_path);
};

#endif // BENCHMARKRUNNER_H
# CMakeLists.txt for MiniDB
cmake_minimum_required(VERSION 3.10)
project(MiniDB)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include directories (so #include "X.h" finds X in any module)
include_directories(
    MiniDB/include
    MiniDB/include/common
    MiniDB/include/storage
    MiniDB/include/buffer
    MiniDB/include/concurrency
    MiniDB/include/log
    MiniDB/include/executor
    MiniDB/include/index
    MiniDB/include/parser
    MiniDB/include/optimizer
    MiniDB/include/benchmark
)

# Source files
set(SOURCES
    MiniDB/src/buffer/BufferPoolManager.cpp
    MiniDB/src/buffer/Clock.cpp
    MiniDB/src/buffer/LayeredClock.cpp
    MiniDB/src/buffer/Page.cpp
    MiniDB/src/buffer/QueryPlanCache.cpp
    MiniDB/src/storage/DiskManager.cpp
    MiniDB/src/storage/table_heap.cpp
    MiniDB/src/storage/LayerManager.cpp
    MiniDB/src/storage/CatalogManager.cpp
    MiniDB/src/index/BPlusTree.cpp
    MiniDB/src/executor/executor.cpp
    MiniDB/src/executor/SeqScanExecutor.cpp
    MiniDB/src/executor/IndexScanExecutor.cpp
    MiniDB/src/executor/InsertExecutor.cpp
    MiniDB/src/executor/MergeExecutor.cpp
    MiniDB/src/executor/JoinExecutor.cpp
    MiniDB/src/executor/AggregationExecutor.cpp
    MiniDB/src/executor/SortExecutor.cpp
    MiniDB/src/concurrency/DeadLockDector.cpp
    MiniDB/src/concurrency/LockManager.cpp
    MiniDB/src/concurrency/ServerManager.cpp
    MiniDB/src/concurrency/Transaction.cpp
    MiniDB/src/concurrency/TransactionManager.cpp
    MiniDB/src/log/LockTable.cpp
    MiniDB/src/log/LogBuffer.cpp
    MiniDB/src/log/WALManager.cpp
    MiniDB/src/log/CheckpointManager.cpp
    MiniDB/src/optimizer/QueryOptimizer.cpp
    MiniDB/src/parser/SQLParser.cpp
    MiniDB/src/benchmark/BenchmarkRunner.cpp
    MiniDB/src/benchmark/MetricsManager.cpp
    MiniDB/src/common/config.cpp
)

# Create main library
add_library(minidb ${SOURCES})

# Test executables
add_executable(buffer_pool_test test/buffer_pool_test.cpp)
target_link_libraries(buffer_pool_test minidb)

add_executable(b_plus_tree_test test/b_plus_tree_test.cpp)
target_link_libraries(b_plus_tree_test minidb)

add_executable(executor_test test/executor_test.cpp)
target_link_libraries(executor_test minidb)

add_executable(lock_manager_test test/lock_manager_test.cpp)
target_link_libraries(lock_manager_test minidb)

add_executable(sql_parser_test test/sql_parser_test.cpp)
target_link_libraries(sql_parser_test minidb)

add_executable(integration_test test/integration_test.cpp)
target_link_libraries(integration_test minidb)

# Enable testing
enable_testing()
add_test(NAME buffer_pool_test COMMAND buffer_pool_test)
add_test(NAME b_plus_tree_test COMMAND b_plus_tree_test)
add_test(NAME executor_test COMMAND executor_test)
add_test(NAME lock_manager_test COMMAND lock_manager_test)
add_test(NAME sql_parser_test COMMAND sql_parser_test)
add_test(NAME integration_test COMMAND integration_test)
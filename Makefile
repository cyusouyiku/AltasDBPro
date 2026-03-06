# MiniDB Makefile（无 CMake 时可直接 make 编译并测试）
CXX = clang++
CXXFLAGS = -std=c++17 -Wall -I MiniDB/include -I MiniDB/include/common -I MiniDB/include/storage \
	-I MiniDB/include/buffer -I MiniDB/include/concurrency -I MiniDB/include/log \
	-I MiniDB/include/executor -I MiniDB/include/index -I MiniDB/include/parser \
	-I MiniDB/include/optimizer -I MiniDB/include/benchmark

BUILD = build
SRC = MiniDB/src
LIB_SRCS = \
	$(SRC)/buffer/BufferPoolManager.cpp $(SRC)/buffer/Clock.cpp $(SRC)/buffer/Page.cpp $(SRC)/buffer/QueryPlanCache.cpp \
	$(SRC)/storage/DiskManager.cpp $(SRC)/storage/table_heap.cpp $(SRC)/storage/CatalogManager.cpp \
	$(SRC)/index/BPlusTree.cpp \
	$(SRC)/executor/executor.cpp $(SRC)/executor/SeqScanExecutor.cpp $(SRC)/executor/IndexScanExecutor.cpp \
	$(SRC)/executor/InsertExecutor.cpp $(SRC)/executor/JoinExecutor.cpp $(SRC)/executor/AggregationExecutor.cpp $(SRC)/executor/SortExecutor.cpp \
	$(SRC)/concurrency/DeadLockDector.cpp $(SRC)/concurrency/LockManager.cpp $(SRC)/concurrency/ServerManager.cpp \
	$(SRC)/concurrency/Transaction.cpp $(SRC)/concurrency/TransactionManager.cpp \
	$(SRC)/log/LockTable.cpp $(SRC)/log/LogBuffer.cpp $(SRC)/log/WALManager.cpp $(SRC)/log/CheckpointManager.cpp \
	$(SRC)/optimizer/QueryOptimizer.cpp $(SRC)/parser/SQLParser.cpp $(SRC)/benchmark/BenchmarkRunner.cpp $(SRC)/common/config.cpp

LIB_OBJS = $(patsubst $(SRC)/%.cpp,$(BUILD)/%.o,$(LIB_SRCS))
LIB = $(BUILD)/libminidb.a

TESTS = buffer_pool_test b_plus_tree_test executor_test lock_manager_test sql_parser_test integration_test

.PHONY: all clean test

all: $(LIB) $(addprefix $(BUILD)/,$(TESTS))

$(BUILD)/%.o: $(SRC)/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c -o $@ $<

$(LIB): $(LIB_OBJS)
	ar rcs $@ $^

$(BUILD)/buffer_pool_test: test/buffer_pool_test.cpp $(LIB)
	$(CXX) $(CXXFLAGS) -o $@ $< $(LIB)
$(BUILD)/b_plus_tree_test: test/b_plus_tree_test.cpp $(LIB)
	$(CXX) $(CXXFLAGS) -o $@ $< $(LIB)
$(BUILD)/executor_test: test/executor_test.cpp $(LIB)
	$(CXX) $(CXXFLAGS) -o $@ $< $(LIB)
$(BUILD)/lock_manager_test: test/lock_manager_test.cpp $(LIB)
	$(CXX) $(CXXFLAGS) -o $@ $< $(LIB)
$(BUILD)/sql_parser_test: test/sql_parser_test.cpp $(LIB)
	$(CXX) $(CXXFLAGS) -o $@ $< $(LIB)
$(BUILD)/integration_test: test/integration_test.cpp $(LIB)
	$(CXX) $(CXXFLAGS) -o $@ $< $(LIB)

test: all
	@echo "========== buffer_pool_test =========="
	@$(BUILD)/buffer_pool_test
	@echo "========== b_plus_tree_test =========="
	@$(BUILD)/b_plus_tree_test
	@echo "========== executor_test =========="
	@$(BUILD)/executor_test
	@echo "========== lock_manager_test =========="
	@$(BUILD)/lock_manager_test
	@echo "========== sql_parser_test =========="
	@$(BUILD)/sql_parser_test
	@echo "========== integration_test =========="
	@$(BUILD)/integration_test
	@echo "========== 全部测试通过 =========="

clean:
	rm -rf $(BUILD)

//
//  SQLParser.h
//  MiniDB
//
//  Created by Framework Generator on 2026/1/24.
//

#ifndef SQLPARSER_H
#define SQLPARSER_H

#include <string>
#include <vector>
#include <map>

// SQL 语句类型枚举
enum class SQLStatementType {
    SELECT,
    INSERT,
    UPDATE,
    DELETE,
    CREATE_TABLE,
    UNKNOWN
};

// 解析结果结构
struct ParseResult {
    SQLStatementType type;
    std::map<std::string, std::string> attributes;  // 存储解析出的各种属性
    std::vector<std::string> tokens;  // 词法分析结果
    bool success;
    std::string error_message;
    
    ParseResult() : type(SQLStatementType::UNKNOWN), success(false) {}
};

class SQLParser {
private:
    // 辅助方法：词法分析
    std::vector<std::string> Tokenize(const std::string& sql);
    
    // 辅助方法：转换为大写
    std::string ToUpper(const std::string& str);
    
    // 辅助方法：去除首尾空白
    std::string Trim(const std::string& str);
    
    // 语法分析方法
    ParseResult ParseSelect(const std::vector<std::string>& tokens, size_t& pos);
    ParseResult ParseInsert(const std::vector<std::string>& tokens, size_t& pos);
    ParseResult ParseUpdate(const std::vector<std::string>& tokens, size_t& pos);
    ParseResult ParseDelete(const std::vector<std::string>& tokens, size_t& pos);
    ParseResult ParseCreateTable(const std::vector<std::string>& tokens, size_t& pos);
    
    // 辅助方法：解析列名列表
    std::vector<std::string> ParseColumnList(const std::vector<std::string>& tokens, size_t& pos);
    
    // 辅助方法：解析值列表
    std::vector<std::string> ParseValueList(const std::vector<std::string>& tokens, size_t& pos);
    
    // 辅助方法：解析 WHERE 子句
    void ParseWhereClause(const std::vector<std::string>& tokens, size_t& pos, ParseResult& result);
    
    // 辅助方法：解析表名
    std::string ParseTableName(const std::vector<std::string>& tokens, size_t& pos);

public:
    SQLParser();
    ~SQLParser();

    // 主解析方法
    ParseResult ParseQuery(const std::string& sql);
};

#endif 

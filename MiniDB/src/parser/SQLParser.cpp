//
//  SQLParser.cpp
//  MiniDB
//
//  Created by Framework Generator on 2026/1/24.
//

#include "SQLParser.h"
#include <cctype>
#include <sstream>
#include <algorithm>
#include <cstring>

SQLParser::SQLParser() {}

SQLParser::~SQLParser() {}

// 转换为大写
std::string SQLParser::ToUpper(const std::string& str) {
    std::string result = str;
    std::transform(result.begin(), result.end(), result.begin(), 
                   [](unsigned char c) { return std::toupper(c); });
    return result;
}

// 去除首尾空白
std::string SQLParser::Trim(const std::string& str) {
    size_t first = str.find_first_not_of(" \t\n\r");
    if (first == std::string::npos) {
        return "";
    }
    size_t last = str.find_last_not_of(" \t\n\r");
    return str.substr(first, (last - first + 1));
}

// 词法分析：将 SQL 字符串分解为 tokens
std::vector<std::string> SQLParser::Tokenize(const std::string& sql) {
    std::vector<std::string> tokens;
    std::string current;
    bool in_quotes = false;
    char quote_char = '\0';
    
    for (size_t i = 0; i < sql.length(); ++i) {
        char c = sql[i];
        
        // 处理引号内的字符串
        if (c == '\'' || c == '"') {
            if (!in_quotes) {
                in_quotes = true;
                quote_char = c;
                if (!current.empty()) {
                    tokens.push_back(current);
                    current.clear();
                }
                current += c;
            } else if (c == quote_char) {
                // 检查是否是转义的引号
                if (i + 1 < sql.length() && sql[i + 1] == quote_char) {
                    current += c;
                    current += c;
                    ++i;  // 跳过下一个引号
                } else {
                    in_quotes = false;
                    current += c;
                    tokens.push_back(current);
                    current.clear();
                }
            } else {
                current += c;
            }
            continue;
        }
        
        if (in_quotes) {
            current += c;
            continue;
        }
        
        // 处理空白字符
        if (std::isspace(static_cast<unsigned char>(c))) {
            if (!current.empty()) {
                tokens.push_back(current);
                current.clear();
            }
            continue;
        }
        
        // 处理特殊字符（分隔符）
        if (c == ',' || c == ';' || c == '(' || c == ')' || c == '=' || 
            c == '<' || c == '>' || c == '!') {
            if (!current.empty()) {
                tokens.push_back(current);
                current.clear();
            }
            
            // 处理多字符操作符（如 <=, >=, !=）
            if ((c == '<' || c == '>' || c == '!') && 
                i + 1 < sql.length() && sql[i + 1] == '=') {
                std::string op;
                op += c;
                op += '=';
                tokens.push_back(op);
                ++i;  // 跳过 '='
            } else {
                tokens.push_back(std::string(1, c));
            }
            continue;
        }
        
        // 普通字符
        current += c;
    }
    
    // 处理最后一个 token
    if (!current.empty()) {
        tokens.push_back(current);
    }
    
    // 移除空 tokens
    tokens.erase(std::remove_if(tokens.begin(), tokens.end(),
                                [](const std::string& s) { return s.empty(); }),
                 tokens.end());
    
    return tokens;
}

// 解析表名
std::string SQLParser::ParseTableName(const std::vector<std::string>& tokens, size_t& pos) {
    if (pos >= tokens.size()) {
        return "";
    }
    std::string table_name = tokens[pos];
    pos++;
    return table_name;
}

// 解析列名列表（如 SELECT col1, col2, col3 或 INSERT INTO table (col1, col2)）
std::vector<std::string> SQLParser::ParseColumnList(const std::vector<std::string>& tokens, size_t& pos) {
    std::vector<std::string> columns;
    
    if (pos >= tokens.size()) {
        return columns;
    }
    
    // 处理 * (SELECT *)
    if (tokens[pos] == "*") {
        columns.push_back("*");
        pos++;
        return columns;
    }
    
    // 跳过开括号（如果有）
    if (pos < tokens.size() && tokens[pos] == "(") {
        pos++;
    }
    
    while (pos < tokens.size()) {
        std::string token = tokens[pos];
        
        if (token == ")" || token == ",") {
            if (token == ")") {
                pos++;
                break;
            }
            pos++;
            continue;
        }
        
        // 跳过关键字
        std::string upper_token = ToUpper(token);
        if (upper_token == "FROM" || upper_token == "WHERE" || 
            upper_token == "VALUES" || upper_token == "SET") {
            break;
        }
        
        columns.push_back(token);
        pos++;
        
        // 检查是否有逗号
        if (pos < tokens.size() && tokens[pos] == ",") {
            pos++;
        } else if (pos < tokens.size() && tokens[pos] == ")") {
            pos++;
            break;
        }
    }
    
    return columns;
}

// 解析值列表（如 INSERT INTO table VALUES (val1, val2, val3)）
std::vector<std::string> SQLParser::ParseValueList(const std::vector<std::string>& tokens, size_t& pos) {
    std::vector<std::string> values;
    
    if (pos >= tokens.size()) {
        return values;
    }
    
    // 跳过 VALUES 关键字（如果有）
    std::string upper_token = ToUpper(tokens[pos]);
    if (upper_token == "VALUES") {
        pos++;
    }
    
    // 跳过开括号
    if (pos < tokens.size() && tokens[pos] == "(") {
        pos++;
    }
    
    while (pos < tokens.size()) {
        std::string token = tokens[pos];
        
        if (token == ")") {
            pos++;
            break;
        }
        
        if (token == ",") {
            pos++;
            continue;
        }
        
        values.push_back(token);
        pos++;
    }
    
    return values;
}

// 解析 WHERE 子句
void SQLParser::ParseWhereClause(const std::vector<std::string>& tokens, size_t& pos, ParseResult& result) {
    if (pos >= tokens.size()) {
        return;
    }
    
    std::string upper_token = ToUpper(tokens[pos]);
    if (upper_token != "WHERE") {
        return;
    }
    
    pos++;  // 跳过 WHERE
    
    if (pos >= tokens.size()) {
        result.success = false;
        result.error_message = "WHERE clause is incomplete";
        return;
    }
    
    // 解析条件：column operator value
    std::string column = tokens[pos++];
    result.attributes["where_column"] = column;
    
    if (pos >= tokens.size()) {
        result.success = false;
        result.error_message = "WHERE clause operator missing";
        return;
    }
    
    std::string op = tokens[pos++];
    result.attributes["where_operator"] = op;
    
    if (pos >= tokens.size()) {
        result.success = false;
        result.error_message = "WHERE clause value missing";
        return;
    }
    
    std::string value = tokens[pos++];
    result.attributes["where_value"] = value;
}

// 解析 SELECT 语句
ParseResult SQLParser::ParseSelect(const std::vector<std::string>& tokens, size_t& pos) {
    ParseResult result;
    result.type = SQLStatementType::SELECT;
    result.success = true;
    
    pos++;  // 跳过 SELECT
    
    // 解析列列表
    std::vector<std::string> columns = ParseColumnList(tokens, pos);
    if (columns.empty()) {
        result.success = false;
        result.error_message = "SELECT statement must have at least one column";
        return result;
    }
    
    // 将列列表转换为字符串存储
    std::string columns_str;
    for (size_t i = 0; i < columns.size(); ++i) {
        if (i > 0) columns_str += ",";
        columns_str += columns[i];
    }
    result.attributes["columns"] = columns_str;
    
    // 解析 FROM 子句
    if (pos >= tokens.size()) {
        result.success = false;
        result.error_message = "SELECT statement missing FROM clause";
        return result;
    }
    
    std::string upper_token = ToUpper(tokens[pos]);
    if (upper_token != "FROM") {
        result.success = false;
        result.error_message = "SELECT statement missing FROM keyword";
        return result;
    }
    
    pos++;  // 跳过 FROM
    
    // 解析表名
    std::string table_name = ParseTableName(tokens, pos);
    if (table_name.empty()) {
        result.success = false;
        result.error_message = "SELECT statement missing table name";
        return result;
    }
    result.attributes["table"] = table_name;
    
    // 解析 WHERE 子句（可选）
    if (pos < tokens.size()) {
        ParseWhereClause(tokens, pos, result);
    }
    
    return result;
}

// 解析 INSERT 语句
ParseResult SQLParser::ParseInsert(const std::vector<std::string>& tokens, size_t& pos) {
    ParseResult result;
    result.type = SQLStatementType::INSERT;
    result.success = true;
    
    pos++;  // 跳过 INSERT
    
    // 解析 INTO 关键字
    if (pos >= tokens.size()) {
        result.success = false;
        result.error_message = "INSERT statement incomplete";
        return result;
    }
    
    std::string upper_token = ToUpper(tokens[pos]);
    if (upper_token != "INTO") {
        result.success = false;
        result.error_message = "INSERT statement missing INTO keyword";
        return result;
    }
    
    pos++;  // 跳过 INTO
    
    // 解析表名
    std::string table_name = ParseTableName(tokens, pos);
    if (table_name.empty()) {
        result.success = false;
        result.error_message = "INSERT statement missing table name";
        return result;
    }
    result.attributes["table"] = table_name;
    
    // 解析列列表（可选）
    if (pos < tokens.size() && tokens[pos] == "(") {
        std::vector<std::string> columns = ParseColumnList(tokens, pos);
        std::string columns_str;
        for (size_t i = 0; i < columns.size(); ++i) {
            if (i > 0) columns_str += ",";
            columns_str += columns[i];
        }
        result.attributes["columns"] = columns_str;
    }
    
    // 解析 VALUES 子句
    if (pos >= tokens.size()) {
        result.success = false;
        result.error_message = "INSERT statement missing VALUES clause";
        return result;
    }
    
    std::vector<std::string> values = ParseValueList(tokens, pos);
    if (values.empty()) {
        result.success = false;
        result.error_message = "INSERT statement missing values";
        return result;
    }
    
    std::string values_str;
    for (size_t i = 0; i < values.size(); ++i) {
        if (i > 0) values_str += ",";
        values_str += values[i];
    }
    result.attributes["values"] = values_str;
    
    return result;
}

// 解析 UPDATE 语句
ParseResult SQLParser::ParseUpdate(const std::vector<std::string>& tokens, size_t& pos) {
    ParseResult result;
    result.type = SQLStatementType::UPDATE;
    result.success = true;
    
    pos++;  // 跳过 UPDATE
    
    // 解析表名
    std::string table_name = ParseTableName(tokens, pos);
    if (table_name.empty()) {
        result.success = false;
        result.error_message = "UPDATE statement missing table name";
        return result;
    }
    result.attributes["table"] = table_name;
    
    // 解析 SET 子句
    if (pos >= tokens.size()) {
        result.success = false;
        result.error_message = "UPDATE statement missing SET clause";
        return result;
    }
    
    std::string upper_token = ToUpper(tokens[pos]);
    if (upper_token != "SET") {
        result.success = false;
        result.error_message = "UPDATE statement missing SET keyword";
        return result;
    }
    
    pos++;  // 跳过 SET
    
    // 解析 SET column = value
    if (pos >= tokens.size()) {
        result.success = false;
        result.error_message = "UPDATE SET clause incomplete";
        return result;
    }
    
    std::string column = tokens[pos++];
    result.attributes["set_column"] = column;
    
    if (pos >= tokens.size() || tokens[pos] != "=") {
        result.success = false;
        result.error_message = "UPDATE SET clause missing = operator";
        return result;
    }
    
    pos++;  // 跳过 =
    
    if (pos >= tokens.size()) {
        result.success = false;
        result.error_message = "UPDATE SET clause missing value";
        return result;
    }
    
    std::string value = tokens[pos++];
    result.attributes["set_value"] = value;
    
    // 解析 WHERE 子句（可选但通常需要）
    if (pos < tokens.size()) {
        ParseWhereClause(tokens, pos, result);
    }
    
    return result;
}

// 解析 DELETE 语句
ParseResult SQLParser::ParseDelete(const std::vector<std::string>& tokens, size_t& pos) {
    ParseResult result;
    result.type = SQLStatementType::DELETE;
    result.success = true;
    
    pos++;  // 跳过 DELETE
    
    // 解析 FROM 关键字
    if (pos >= tokens.size()) {
        result.success = false;
        result.error_message = "DELETE statement incomplete";
        return result;
    }
    
    std::string upper_token = ToUpper(tokens[pos]);
    if (upper_token == "FROM") {
        pos++;  // 跳过 FROM（可选）
    }
    
    // 解析表名
    std::string table_name = ParseTableName(tokens, pos);
    if (table_name.empty()) {
        result.success = false;
        result.error_message = "DELETE statement missing table name";
        return result;
    }
    result.attributes["table"] = table_name;
    
    // 解析 WHERE 子句（可选但通常需要）
    if (pos < tokens.size()) {
        ParseWhereClause(tokens, pos, result);
    }
    
    return result;
}

// 解析 CREATE TABLE 语句
ParseResult SQLParser::ParseCreateTable(const std::vector<std::string>& tokens, size_t& pos) {
    ParseResult result;
    result.type = SQLStatementType::CREATE_TABLE;
    result.success = true;
    
    pos++;  // 跳过 CREATE
    
    // 解析 TABLE 关键字
    if (pos >= tokens.size()) {
        result.success = false;
        result.error_message = "CREATE statement incomplete";
        return result;
    }
    
    std::string upper_token = ToUpper(tokens[pos]);
    if (upper_token != "TABLE") {
        result.success = false;
        result.error_message = "CREATE statement missing TABLE keyword";
        return result;
    }
    
    pos++;  // 跳过 TABLE
    
    // 解析表名
    std::string table_name = ParseTableName(tokens, pos);
    if (table_name.empty()) {
        result.success = false;
        result.error_message = "CREATE TABLE statement missing table name";
        return result;
    }
    result.attributes["table"] = table_name;
    
    // 解析列定义（简化版：只解析列名，不解析类型）
    if (pos >= tokens.size() || tokens[pos] != "(") {
        result.success = false;
        result.error_message = "CREATE TABLE statement missing column definitions";
        return result;
    }
    
    pos++;  // 跳过 (
    
    std::vector<std::string> columns;
    while (pos < tokens.size() && tokens[pos] != ")") {
        std::string token = tokens[pos];
        
        if (token == ",") {
            pos++;
            continue;
        }
        
        // 简化：只提取列名，忽略类型定义
        std::string upper = ToUpper(token);
        if (upper == "INT" || upper == "VARCHAR" || upper == "TEXT" || 
            upper == "CHAR" || upper == "INTEGER" || upper == "PRIMARY" || 
            upper == "KEY" || upper == "NOT" || upper == "NULL") {
            // 跳过类型关键字
            pos++;
            // 如果是 VARCHAR(10) 这样的，跳过括号和数字
            if (pos < tokens.size() && tokens[pos] == "(") {
                pos++;  // 跳过 (
                while (pos < tokens.size() && tokens[pos] != ")") {
                    pos++;
                }
                if (pos < tokens.size() && tokens[pos] == ")") {
                    pos++;  // 跳过 )
                }
            }
            continue;
        }
        
        columns.push_back(token);
        pos++;
    }
    
    if (pos < tokens.size() && tokens[pos] == ")") {
        pos++;  // 跳过 )
    }
    
    if (columns.empty()) {
        result.success = false;
        result.error_message = "CREATE TABLE statement has no columns";
        return result;
    }
    
    std::string columns_str;
    for (size_t i = 0; i < columns.size(); ++i) {
        if (i > 0) columns_str += ",";
        columns_str += columns[i];
    }
    result.attributes["columns"] = columns_str;
    
    return result;
}

// 主解析方法
ParseResult SQLParser::ParseQuery(const std::string& sql) {
    ParseResult result;
    
    // 词法分析
    result.tokens = Tokenize(sql);
    
    if (result.tokens.empty()) {
        result.success = false;
        result.error_message = "Empty SQL statement";
        return result;
    }
    
    // 确定语句类型
    size_t pos = 0;
    std::string first_token = ToUpper(result.tokens[0]);
    
    if (first_token == "SELECT") {
        result = ParseSelect(result.tokens, pos);
    } else if (first_token == "INSERT") {
        result = ParseInsert(result.tokens, pos);
    } else if (first_token == "UPDATE") {
        result = ParseUpdate(result.tokens, pos);
    } else if (first_token == "DELETE") {
        result = ParseDelete(result.tokens, pos);
    } else if (first_token == "CREATE") {
        if (result.tokens.size() > 1 && ToUpper(result.tokens[1]) == "TABLE") {
            result = ParseCreateTable(result.tokens, pos);
        } else {
            result.success = false;
            result.error_message = "Unsupported CREATE statement type";
        }
    } else {
        result.success = false;
        result.error_message = "Unsupported SQL statement type: " + result.tokens[0];
    }
    
    // 确保 tokens 被保留
    if (result.tokens.empty()) {
        result.tokens = Tokenize(sql);
    }
    
    return result;
}

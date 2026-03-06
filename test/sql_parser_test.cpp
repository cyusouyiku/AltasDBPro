//
//  sql_parser_test.cpp
//  MiniDB
//
//  Test file for SQLParser
//

#include "parser/SQLParser.h"
#include <iostream>
#include <cassert>

void TestSelect() {
    std::cout << "Testing SELECT statement...\n";
    SQLParser parser;
    
    ParseResult result = parser.ParseQuery("SELECT * FROM users");
    assert(result.success);
    assert(result.type == SQLStatementType::SELECT);
    assert(result.attributes["columns"] == "*");
    assert(result.attributes["table"] == "users");
    std::cout << "  ✓ SELECT * FROM users\n";
    
    result = parser.ParseQuery("SELECT id, name, email FROM users WHERE id = 1");
    assert(result.success);
    assert(result.type == SQLStatementType::SELECT);
    assert(result.attributes["table"] == "users");
    assert(result.attributes["where_column"] == "id");
    assert(result.attributes["where_operator"] == "=");
    assert(result.attributes["where_value"] == "1");
    std::cout << "  ✓ SELECT with WHERE clause\n";
}

void TestInsert() {
    std::cout << "Testing INSERT statement...\n";
    SQLParser parser;
    
    ParseResult result = parser.ParseQuery("INSERT INTO users VALUES (1, 'John', 'john@example.com')");
    assert(result.success);
    assert(result.type == SQLStatementType::INSERT);
    assert(result.attributes["table"] == "users");
    std::cout << "  ✓ INSERT INTO users VALUES\n";
    
    result = parser.ParseQuery("INSERT INTO users (id, name) VALUES (1, 'John')");
    assert(result.success);
    assert(result.type == SQLStatementType::INSERT);
    assert(result.attributes["table"] == "users");
    assert(result.attributes.find("columns") != result.attributes.end());
    std::cout << "  ✓ INSERT with column list\n";
}

void TestUpdate() {
    std::cout << "Testing UPDATE statement...\n";
    SQLParser parser;
    
    ParseResult result = parser.ParseQuery("UPDATE users SET name = 'Jane' WHERE id = 1");
    assert(result.success);
    assert(result.type == SQLStatementType::UPDATE);
    assert(result.attributes["table"] == "users");
    assert(result.attributes["set_column"] == "name");
    assert(result.attributes["set_value"] == "'Jane'");
    assert(result.attributes["where_column"] == "id");
    std::cout << "  ✓ UPDATE with WHERE clause\n";
}

void TestDelete() {
    std::cout << "Testing DELETE statement...\n";
    SQLParser parser;
    
    ParseResult result = parser.ParseQuery("DELETE FROM users WHERE id = 1");
    assert(result.success);
    assert(result.type == SQLStatementType::DELETE);
    assert(result.attributes["table"] == "users");
    assert(result.attributes["where_column"] == "id");
    std::cout << "  ✓ DELETE with WHERE clause\n";
}

void TestCreateTable() {
    std::cout << "Testing CREATE TABLE statement...\n";
    SQLParser parser;
    
    ParseResult result = parser.ParseQuery("CREATE TABLE users (id INT, name VARCHAR(50), email TEXT)");
    assert(result.success);
    assert(result.type == SQLStatementType::CREATE_TABLE);
    assert(result.attributes["table"] == "users");
    assert(result.attributes.find("columns") != result.attributes.end());
    std::cout << "  ✓ CREATE TABLE\n";
}

int main() {
    std::cout << "=== SQL Parser Test ===\n\n";
    
    try {
        TestSelect();
        TestInsert();
        TestUpdate();
        TestDelete();
        TestCreateTable();
        
        std::cout << "\n✓ All tests passed!\n";
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "✗ Test failed: " << e.what() << "\n";
        return 1;
    }
}

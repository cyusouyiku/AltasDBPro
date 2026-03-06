//
//  CatalogManager.cpp
//  MiniDB
//
//  Created by Framework Generator on 2026/1/24.
//

#include "CatalogManager.h"

CatalogManager::CatalogManager() {}

CatalogManager::~CatalogManager() {}

bool CatalogManager::CreateTable(const std::string& table_name) {
    if (table_name.empty()) return false;
    if (tables_.count(table_name)) return false;
    tables_.insert(table_name);
    return true;
}

bool CatalogManager::DropTable(const std::string& table_name) {
    if (table_name.empty()) return false;
    auto it = tables_.find(table_name);
    if (it == tables_.end()) return false;
    tables_.erase(it);
    return true;
}

bool CatalogManager::TableExists(const std::string& table_name) {
    return tables_.count(table_name) > 0;
}
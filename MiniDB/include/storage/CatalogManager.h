//
//  CatalogManager.h
//  MiniDB
//
//  Created by Framework Generator on 2026/1/24.
//

#ifndef CATALOGMANAGER_H
#define CATALOGMANAGER_H

#include <string>
#include <unordered_set>

class CatalogManager {
private:
    std::unordered_set<std::string> tables_;

public:
    CatalogManager();
    ~CatalogManager();

    bool CreateTable(const std::string& table_name);
    bool DropTable(const std::string& table_name);
    bool TableExists(const std::string& table_name);
};

#endif // CATALOGMANAGER_H
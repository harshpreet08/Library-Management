#pragma once

#include <sqlite3.h>
#include <string>

class DatabaseManager {
public:
    explicit DatabaseManager(const std::string& db_file);
    ~DatabaseManager();

    sqlite3* get();
    void initializeSchema();

private:
    sqlite3* _db = nullptr;
};
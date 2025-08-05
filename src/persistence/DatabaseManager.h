#pragma once
#include <sqlite3.h>
#include <string>

class DatabaseManager {
public:
    explicit DatabaseManager(const std::string& dbPath);
    ~DatabaseManager();

    sqlite3* get();
    void initializeSchema();

private:
    sqlite3* _db = nullptr;
};
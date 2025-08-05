#include "DatabaseManager.h"
#include <stdexcept>

DatabaseManager::DatabaseManager(const std::string& dbPath) {
    if (sqlite3_open(dbPath.c_str(), &_db) != SQLITE_OK) {
        throw std::runtime_error("Cannot open database: " + std::string(sqlite3_errmsg(_db)));
    }
}

DatabaseManager::~DatabaseManager() {
    if (_db) sqlite3_close(_db);
}

sqlite3* DatabaseManager::get() {
    return _db;
}

void DatabaseManager::initializeSchema() {
    const char* sql = R"(
        CREATE TABLE IF NOT EXISTS users (
            id            TEXT PRIMARY KEY,
            name          TEXT NOT NULL,
            role          TEXT NOT NULL DEFAULT 'user',
            password_hash TEXT NOT NULL
        );
        CREATE TABLE IF NOT EXISTS assets (
            id              TEXT PRIMARY KEY,
            type            TEXT NOT NULL,
            title           TEXT NOT NULL,
            author_or_owner TEXT NOT NULL,
            is_issued       INTEGER NOT NULL DEFAULT 0
        );
        CREATE TABLE IF NOT EXISTS loans (
            asset_id   TEXT PRIMARY KEY,
            user_id    TEXT NOT NULL,
            issue_date INTEGER,
            FOREIGN KEY(asset_id) REFERENCES assets(id),
            FOREIGN KEY(user_id)  REFERENCES users(id)
        );
    )";
    char* err = nullptr;
    if (sqlite3_exec(_db, sql, nullptr, nullptr, &err) != SQLITE_OK) {
        std::string e = err ? err : "unknown";
        sqlite3_free(err);
        throw std::runtime_error("Schema init failed: " + e);
    }
}
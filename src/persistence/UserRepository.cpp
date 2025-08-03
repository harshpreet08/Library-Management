#include "UserRepository.h"
#include <sqlite3.h>
#include <stdexcept>

UserRepository::UserRepository(std::shared_ptr<DatabaseManager> db)
    : _db(std::move(db)) {}

void UserRepository::add(const User& user) {
    const char* sql = "INSERT OR IGNORE INTO users (id, name) VALUES (?, ?);";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(_db->get(), sql, -1, &stmt, nullptr) != SQLITE_OK)
        throw std::runtime_error("Prepare user insert failed");

    sqlite3_bind_text(stmt, 1, user.id().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, user.name().c_str(), -1, SQLITE_TRANSIENT);

    if (sqlite3_step(stmt) != SQLITE_DONE) {
        sqlite3_finalize(stmt);
        throw std::runtime_error("User insert failed");
    }
    sqlite3_finalize(stmt);
}

std::optional<User> UserRepository::find(const std::string& id) {
    const char* sql = "SELECT name FROM users WHERE id = ?;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(_db->get(), sql, -1, &stmt, nullptr) != SQLITE_OK)
        return std::nullopt;

    sqlite3_bind_text(stmt, 1, id.c_str(), -1, SQLITE_TRANSIENT);
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        std::string name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        sqlite3_finalize(stmt);
        return User(id, name);
    }
    sqlite3_finalize(stmt);
    return std::nullopt;
}

std::vector<User> UserRepository::getAll() {
    const char* sql = "SELECT id, name FROM users;";
    sqlite3_stmt* stmt = nullptr;
    std::vector<User> out;
    if (sqlite3_prepare_v2(_db->get(), sql, -1, &stmt, nullptr) != SQLITE_OK)
        return out;

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        std::string id = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        std::string name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        out.emplace_back(id, name);
    }
    sqlite3_finalize(stmt);
    return out;
}
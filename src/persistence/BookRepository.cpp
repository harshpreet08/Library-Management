#include "BookRepository.h"
#include <sqlite3.h>
#include <stdexcept>
#include <ctime>

BookRepository::BookRepository(std::shared_ptr<DatabaseManager> db)
    : _db(std::move(db)) {}

void BookRepository::add(const Book& book) {
    const char* sql = "INSERT OR IGNORE INTO books (id, title, author, is_issued) VALUES (?, ?, ?, ?);";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(_db->get(), sql, -1, &stmt, nullptr) != SQLITE_OK)
        throw std::runtime_error("Prepare failed");

    sqlite3_bind_text(stmt, 1, book.id().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, book.title().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, book.author().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 4, book.isIssued() ? 1 : 0);

    if (sqlite3_step(stmt) != SQLITE_DONE) {
        sqlite3_finalize(stmt);
        throw std::runtime_error("Insert failed");
    }
    sqlite3_finalize(stmt);
}

std::unique_ptr<Book> BookRepository::find(const std::string& id) {
    const char* sql = "SELECT title, author, is_issued FROM books WHERE id = ?;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(_db->get(), sql, -1, &stmt, nullptr) != SQLITE_OK)
        return nullptr;

    sqlite3_bind_text(stmt, 1, id.c_str(), -1, SQLITE_TRANSIENT);
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        std::string title = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        std::string author = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        bool issued = sqlite3_column_int(stmt, 2) != 0;
        sqlite3_finalize(stmt);
        auto book = std::make_unique<Book>(id, title, author);
        book->setIssued(issued);
        return book;
    }
    sqlite3_finalize(stmt);
    return nullptr;
}

std::vector<Book> BookRepository::getAll() {
    const char* sql = "SELECT id, title, author, is_issued FROM books;";
    sqlite3_stmt* stmt = nullptr;
    std::vector<Book> out;
    if (sqlite3_prepare_v2(_db->get(), sql, -1, &stmt, nullptr) != SQLITE_OK)
        return out;

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        std::string id = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        std::string title = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        std::string author = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        bool issued = sqlite3_column_int(stmt, 3) != 0;
        Book b(id, title, author);
        b.setIssued(issued);
        out.push_back(std::move(b));
    }
    sqlite3_finalize(stmt);
    return out;
}

void BookRepository::setIssued(const std::string& id, bool issued) {
    const char* sql = "UPDATE books SET is_issued = ? WHERE id = ?;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(_db->get(), sql, -1, &stmt, nullptr) != SQLITE_OK)
        throw std::runtime_error("Prepare failed");

    sqlite3_bind_int(stmt, 1, issued ? 1 : 0);
    sqlite3_bind_text(stmt, 2, id.c_str(), -1, SQLITE_TRANSIENT);

    if (sqlite3_step(stmt) != SQLITE_DONE) {
        sqlite3_finalize(stmt);
        throw std::runtime_error("Update failed");
    }
    sqlite3_finalize(stmt);
}

bool BookRepository::isIssued(const std::string& id) {
    const char* sql = "SELECT is_issued FROM books WHERE id = ?;";
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(_db->get(), sql, -1, &stmt, nullptr) != SQLITE_OK)
        return false;

    sqlite3_bind_text(stmt, 1, id.c_str(), -1, SQLITE_TRANSIENT);
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        bool issued = sqlite3_column_int(stmt, 0) != 0;
        sqlite3_finalize(stmt);
        return issued;
    }
    sqlite3_finalize(stmt);
    return false;
}
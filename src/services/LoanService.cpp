#include "LoanService.h"
#include <sqlite3.h>
#include <iostream>
#include <ctime>
#include <stdexcept>

LoanService::LoanService(std::shared_ptr<BookRepository> repo, std::shared_ptr<UserRepository> userRepo)
    : _repo(std::move(repo)), _userRepo(std::move(userRepo)) {}

void LoanService::setLoan(const std::string& bookId, const std::string& userId) {
    const char* sql = R"(
        INSERT OR REPLACE INTO loans (book_id, user_id, issue_date)
        VALUES (?, ?, ?);
    )";
    sqlite3* db = _repo->getDb()->get();
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK)
        throw std::runtime_error("Loan prepare failed");

    time_t now = std::time(nullptr);
    sqlite3_bind_text(stmt, 1, bookId.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, userId.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int64(stmt, 3, static_cast<sqlite3_int64>(now));

    if (sqlite3_step(stmt) != SQLITE_DONE) {
        sqlite3_finalize(stmt);
        throw std::runtime_error("Loan insert failed");
    }
    sqlite3_finalize(stmt);
}

void LoanService::clearLoan(const std::string& bookId) {
    const char* sql = "DELETE FROM loans WHERE book_id = ?;";
    sqlite3* db = _repo->getDb()->get();
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK)
        return;

    sqlite3_bind_text(stmt, 1, bookId.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
}

time_t LoanService::getIssueDate(const std::string& bookId) {
    const char* sql = "SELECT issue_date FROM loans WHERE book_id = ?;";
    sqlite3* db = _repo->getDb()->get();
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK)
        return 0;

    sqlite3_bind_text(stmt, 1, bookId.c_str(), -1, SQLITE_TRANSIENT);
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        sqlite3_int64 v = sqlite3_column_int64(stmt, 0);
        sqlite3_finalize(stmt);
        return static_cast<time_t>(v);
    }
    sqlite3_finalize(stmt);
    return 0;
}

bool LoanService::issueBook(const std::string& bookId, const std::string& userId) {
    auto book = _repo->find(bookId);
    if (!book) {
        std::cout << "Book not found\n";
        return false;
    }
    if (book->isIssued()) {
        std::cout << "Already issued\n";
        return false;
    }
    auto userOpt = _userRepo->find(userId);
    if (!userOpt.has_value()) {
        std::cout << "User not found\n";
        return false;
    }
    _repo->setIssued(bookId, true);
    setLoan(bookId, userId);
    std::cout << "Issued to " << userOpt->name() << "\n";
    return true;
}

bool LoanService::returnBook(const std::string& bookId) {
    auto book = _repo->find(bookId);
    if (!book) {
        std::cout << "Book not found\n";
        return false;
    }
    if (!book->isIssued()) {
        std::cout << "Not currently issued\n";
        return false;
    }
    _repo->setIssued(bookId, false);
    clearLoan(bookId);
    std::cout << "Returned\n";
    return true;
}

void LoanService::listAll() {
    auto all = _repo->getAll();
    for (auto& b : all) {
        std::cout << b.id() << " | " << b.title() << " | " << b.author()
                  << " | " << (b.isIssued() ? "Issued" : "Available");
        if (b.isIssued()) {
            time_t issued = getIssueDate(b.id());
            if (issued != 0) {
                time_t now = std::time(nullptr);
                double days = difftime(now, issued) / (60 * 60 * 24);
                std::cout << " | borrowed " << static_cast<int>(days) << " days ago";
            }
        }
        std::cout << "\n";
    }
}

void LoanService::showOverdues() {
    auto all = _repo->getAll();
    time_t now = std::time(nullptr);
    for (auto& b : all) {
        if (!b.isIssued()) continue;
        time_t issued = getIssueDate(b.id());
        if (issued == 0) continue;
        double days = difftime(now, issued) / (60 * 60 * 24);
        if (days > 14) {
            std::cout << "OVERDUE: " << b.id() << " (" << b.title() << ") borrowed "
                      << static_cast<int>(days) << " days ago\n";
        }
    }
}
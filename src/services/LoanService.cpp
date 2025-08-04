#include "LoanService.h"
#include <sqlite3.h>
#include <iostream>
#include <stdexcept>
#include <cmath>

LoanService::LoanService(std::shared_ptr<AssetRepository> assetRepo, std::shared_ptr<UserRepository> userRepo)
    : _assetRepo(std::move(assetRepo)), _userRepo(std::move(userRepo)) {}

void LoanService::setLoan(const std::string& assetId, const std::string& userId) {
    const char* sql = R"(
        INSERT OR REPLACE INTO loans (asset_id, user_id, issue_date)
        VALUES (?, ?, ?);
    )";
    sqlite3* db = _assetRepo->getDb()->get();
    sqlite3_stmt* stmt = nullptr;
    int rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        throw std::runtime_error(std::string("Loan prepare failed: ") + sqlite3_errmsg(db));
    }

    time_t now = std::time(nullptr);
    sqlite3_bind_text(stmt, 1, assetId.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, userId.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int64(stmt, 3, static_cast<sqlite3_int64>(now));

    if (sqlite3_step(stmt) != SQLITE_DONE) {
        std::string err = sqlite3_errmsg(db);
        sqlite3_finalize(stmt);
        throw std::runtime_error("Loan insert failed: " + err);
    }
    sqlite3_finalize(stmt);
}

std::optional<LoanInfo> LoanService::loanInfo(const std::string& assetId) {
    return getLoanInfo(assetId);
}

void LoanService::clearLoan(const std::string& assetId) {
    const char* sql = "DELETE FROM loans WHERE asset_id = ?;";
    sqlite3* db = _assetRepo->getDb()->get();
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK)
        return;

    sqlite3_bind_text(stmt, 1, assetId.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
}

std::optional<LoanInfo> LoanService::getLoanInfo(const std::string& assetId) {
    const char* sql = "SELECT user_id, issue_date FROM loans WHERE asset_id = ?;";
    sqlite3* db = _assetRepo->getDb()->get();
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK)
        return std::nullopt;

    sqlite3_bind_text(stmt, 1, assetId.c_str(), -1, SQLITE_TRANSIENT);
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        std::string userId = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        sqlite3_int64 dateInt = sqlite3_column_int64(stmt, 1);
        sqlite3_finalize(stmt);
        return LoanInfo{userId, static_cast<time_t>(dateInt)};
    }
    sqlite3_finalize(stmt);
    return std::nullopt;
}

bool LoanService::issueAsset(const std::string& assetId, const std::string& userId) {
    auto assetOpt = _assetRepo->find(assetId);
    if (!assetOpt.has_value()) {
        std::cout << "Asset not found\n";
        return false;
    }
    if (assetOpt->isIssued()) {
        std::cout << "Asset is already issued\n";
        return false;
    }
    auto userOpt = _userRepo->find(userId);
    if (!userOpt.has_value()) {
        std::cout << "User not found\n";
        return false;
    }

    sqlite3* db = _assetRepo->getDb()->get();
    char* err = nullptr;
    sqlite3_exec(db, "BEGIN;", nullptr, nullptr, &err);

    try {
        _assetRepo->setIssued(assetId, true);
        setLoan(assetId, userId);
        sqlite3_exec(db, "COMMIT;", nullptr, nullptr, &err);
    } catch (const std::exception& e) {
        sqlite3_exec(db, "ROLLBACK;", nullptr, nullptr, &err);
        std::cout << "Failed to issue: " << e.what() << "\n";
        return false;
    }

    std::cout << "✅ Issued " << assetOpt->title() << " (" << assetTypeToString(assetOpt->type())
              << ") to " << userOpt->name() << ".\n";
    return true;
}

bool LoanService::returnAsset(const std::string& assetId) {
    auto assetOpt = _assetRepo->find(assetId);
    if (!assetOpt.has_value()) {
        std::cout << "Asset not found\n";
        return false;
    }
    if (!assetOpt->isIssued()) {
        std::cout << "Asset is not currently issued\n";
        return false;
    }

    _assetRepo->setIssued(assetId, false);
    clearLoan(assetId);
    std::cout << "✅ Returned " << assetOpt->title() << " (" << assetTypeToString(assetOpt->type()) << ").\n";
    return true;
}

void LoanService::listAll() {
    auto all = _assetRepo->getAll();
    for (auto& a : all) {
        std::cout << a.id() << " | " << assetTypeToString(a.type()) << " | " << a.title()
                  << " | " << a.authorOrOwner() << " | "
                  << (a.isIssued() ? "Issued" : "Available");

        if (a.isIssued()) {
            auto loanInfo = getLoanInfo(a.id());
            if (loanInfo.has_value()) {
                time_t now = std::time(nullptr);
                double days = difftime(now, loanInfo->issueDate) / (60 * 60 * 24);
                std::cout << " | borrowed " << static_cast<int>(std::floor(days)) << " days ago";
                auto userOpt = _userRepo->find(loanInfo->userId);
                if (userOpt.has_value()) {
                    std::cout << " by " << userOpt->name();
                }
            }
        }
        std::cout << "\n";
    }
}

void LoanService::showOverdues() {
    auto all = _assetRepo->getAll();
    time_t now = std::time(nullptr);
    for (auto& a : all) {
        if (!a.isIssued()) continue;
        auto loanInfo = getLoanInfo(a.id());
        if (!loanInfo.has_value()) continue;
        double days = difftime(now, loanInfo->issueDate) / (60 * 60 * 24);
        if (days > 14) {
            std::cout << "⚠️ OVERDUE: " << a.id() << " | " << assetTypeToString(a.type())
                      << " | " << a.title() << " | borrowed " << static_cast<int>(std::floor(days))
                      << " days ago";
            auto userOpt = _userRepo->find(loanInfo->userId);
            if (userOpt.has_value()) {
                std::cout << " by " << userOpt->name();
            }
            std::cout << "\n";
        }
    }
}
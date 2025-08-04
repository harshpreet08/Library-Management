#include "AssetRepository.h"
#include <sqlite3.h>
#include <stdexcept>

AssetRepository::AssetRepository(std::shared_ptr<DatabaseManager> db)
    : _db(std::move(db)) {}

void AssetRepository::add(const Asset& asset) {
    const char* sql = R"(
        INSERT OR IGNORE INTO assets (id, type, title, author_or_owner, is_issued)
        VALUES (?, ?, ?, ?, ?);
    )";
    sqlite3* db = _db->get();
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK)
        throw std::runtime_error("Prepare asset insert failed");

    sqlite3_bind_text(stmt, 1, asset.id().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, assetTypeToString(asset.type()).c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 3, asset.title().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 4, asset.authorOrOwner().c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 5, asset.isIssued() ? 1 : 0);

    if (sqlite3_step(stmt) != SQLITE_DONE) {
        sqlite3_finalize(stmt);
        throw std::runtime_error("Asset insert failed");
    }
    sqlite3_finalize(stmt);
}

std::optional<Asset> AssetRepository::find(const std::string& id) {
    const char* sql = "SELECT type, title, author_or_owner, is_issued FROM assets WHERE id = ?;";
    sqlite3* db = _db->get();
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK)
        return std::nullopt;

    sqlite3_bind_text(stmt, 1, id.c_str(), -1, SQLITE_TRANSIENT);
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        std::string typeStr = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        std::string title = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        std::string authorOrOwner = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        bool issued = sqlite3_column_int(stmt, 3) != 0;
        sqlite3_finalize(stmt);
        Asset asset(id, stringToAssetType(typeStr), title, authorOrOwner);
        asset.setIssued(issued);
        return asset;
    }
    sqlite3_finalize(stmt);
    return std::nullopt;
}

std::vector<Asset> AssetRepository::getAll() {
    const char* sql = "SELECT id, type, title, author_or_owner, is_issued FROM assets;";
    sqlite3* db = _db->get();
    sqlite3_stmt* stmt = nullptr;
    std::vector<Asset> out;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK)
        return out;

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        std::string id = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        std::string typeStr = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
        std::string title = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
        std::string authorOrOwner = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
        bool issued = sqlite3_column_int(stmt, 4) != 0;
        Asset a(id, stringToAssetType(typeStr), title, authorOrOwner);
        a.setIssued(issued);
        out.push_back(std::move(a));
    }
    sqlite3_finalize(stmt);
    return out;
}

void AssetRepository::setIssued(const std::string& id, bool issued) {
    const char* sql = "UPDATE assets SET is_issued = ? WHERE id = ?;";
    sqlite3* db = _db->get();
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK)
        throw std::runtime_error("Prepare update failed");

    sqlite3_bind_int(stmt, 1, issued ? 1 : 0);
    sqlite3_bind_text(stmt, 2, id.c_str(), -1, SQLITE_TRANSIENT);

    if (sqlite3_step(stmt) != SQLITE_DONE) {
        sqlite3_finalize(stmt);
        throw std::runtime_error("Update failed");
    }
    sqlite3_finalize(stmt);
}

bool AssetRepository::isIssued(const std::string& id) {
    const char* sql = "SELECT is_issued FROM assets WHERE id = ?;";
    sqlite3* db = _db->get();
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK)
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
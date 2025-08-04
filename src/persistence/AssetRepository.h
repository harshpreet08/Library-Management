#pragma once
#include "../models/Asset.h"
#include "DatabaseManager.h"
#include <memory>
#include <vector>
#include <optional>

class AssetRepository {
public:
    explicit AssetRepository(std::shared_ptr<DatabaseManager> db);
    void add(const Asset& asset);
    std::optional<Asset> find(const std::string& id);
    std::vector<Asset> getAll();
    void setIssued(const std::string& id, bool issued);
    bool isIssued(const std::string& id);

    std::shared_ptr<DatabaseManager> getDb() const { return _db; }

private:
    std::shared_ptr<DatabaseManager> _db;
};
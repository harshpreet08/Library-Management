#pragma once

#include "../models/User.h"
#include "DatabaseManager.h"
#include <memory>
#include <optional>
#include <vector>

class UserRepository {
public:
    explicit UserRepository(std::shared_ptr<DatabaseManager> db);

    void add(const User& user);
    std::optional<User> find(const std::string& id);
    std::vector<User>   getAll();

private:
    std::shared_ptr<DatabaseManager> _db;
};
#include <gtest/gtest.h>
#include "../persistence/DatabaseManager.h"
#include "../persistence/UserRepository.h"
#include "../models/User.h"
#include "../util/Security.h"

TEST(UserRepositoryTest, AddFind) {
    ASSERT_TRUE(initCrypto());
    auto db = std::make_shared<DatabaseManager>(":memory:");
    db->initializeSchema();
    UserRepository repo(db);

    auto hash = hashPassword("password");
    User u("u1", "Alice", Role::User, hash);
    repo.add(u);

    auto opt = repo.find("u1");
    ASSERT_TRUE(opt.has_value());
    EXPECT_EQ(opt->id(), "u1");
    EXPECT_EQ(opt->name(), "Alice");
    EXPECT_EQ(opt->role(), Role::User);
    EXPECT_TRUE(verifyPassword(opt->passwordHash(), "password"));
}
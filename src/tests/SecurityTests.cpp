#include <gtest/gtest.h>
#include "../util/Security.h"

TEST(SecurityTest, HashAndVerify) {
    ASSERT_TRUE(initCrypto());
    auto hash = hashPassword("test123");
    EXPECT_FALSE(hash.empty());
    EXPECT_TRUE(verifyPassword(hash, "test123"));
    EXPECT_FALSE(verifyPassword(hash, "wrongpw"));
}
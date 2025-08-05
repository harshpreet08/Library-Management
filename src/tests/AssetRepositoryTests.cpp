#include <gtest/gtest.h>
#include "../persistence/DatabaseManager.h"
#include "../persistence/AssetRepository.h"
#include "../models/Asset.h"

TEST(AssetRepositoryTest, AddFind) {
    auto db = std::make_shared<DatabaseManager>(":memory:");
    db->initializeSchema();
    AssetRepository repo(db);

    Asset a("a1", AssetType::Book, "1984", "Orwell");
    repo.add(a);

    auto opt = repo.find("a1");
    ASSERT_TRUE(opt.has_value());
    EXPECT_EQ(opt->id(), "a1");
    EXPECT_EQ(opt->title(), "1984");
    EXPECT_EQ(opt->authorOrOwner(), "Orwell");
    EXPECT_FALSE(opt->isIssued());
}
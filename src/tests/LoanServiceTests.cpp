#include <gtest/gtest.h>
#include "../persistence/DatabaseManager.h"
#include "../persistence/AssetRepository.h"
#include "../persistence/UserRepository.h"
#include "../services/LoanService.h"
#include "../models/Asset.h"
#include "../models/User.h"
#include "../util/Security.h"

TEST(LoanServiceTest, IssueAndReturn) {
    // initialize libsodium
    ASSERT_TRUE(initCrypto());

    // 1) In‐memory DB + repos
    auto db       = std::make_shared<DatabaseManager>(":memory:");
    db->initializeSchema();
    auto assetRepo = std::make_shared<AssetRepository>(db);
    auto userRepo  = std::make_shared<UserRepository>(db);
    LoanService service(assetRepo, userRepo);

    // 2) Add one asset and one user
    Asset a{"A1", AssetType::Book, "Dune", "Herbert"};
    assetRepo->add(a);
    auto hash = hashPassword("pw");
    User u{"U1", "Paul", Role::User, hash};
    userRepo->add(u);

    // 3) Issue the asset
    EXPECT_TRUE(service.issueAsset("A1", "U1"));
    auto maybeA = assetRepo->find("A1");
    ASSERT_TRUE(maybeA.has_value());
    EXPECT_TRUE(maybeA->isIssued());

    // 4) Fetch the loan info (only userId + issueDate exist)
    auto infoOpt = service.loanInfo("A1");
    ASSERT_TRUE(infoOpt.has_value());
    EXPECT_EQ(infoOpt->userId, "U1");
    EXPECT_GT(infoOpt->issueDate, 0) << "issueDate should be a positive timestamp";

    // 5) Return the asset
    EXPECT_NO_THROW(service.returnAsset("A1"));
    maybeA = assetRepo->find("A1");
    ASSERT_TRUE(maybeA.has_value());
    EXPECT_FALSE(maybeA->isIssued());
}
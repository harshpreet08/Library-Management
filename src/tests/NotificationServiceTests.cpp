#include <gtest/gtest.h>
#include "../persistence/DatabaseManager.h"
#include "../persistence/AssetRepository.h"
#include "../persistence/UserRepository.h"
#include "../services/NotificationService.h"
#include "../services/LoanService.h"
#include "../models/Asset.h"
#include "../models/User.h"
#include "../util/Security.h"
#include <sqlite3.h>

TEST(NotificationServiceTest, CountOverdue) {
    ASSERT_TRUE(initCrypto());

    // 1) Setup DB, repos, services
    auto db = std::make_shared<DatabaseManager>(":memory:");
    db->initializeSchema();
    auto assetRepo = std::make_shared<AssetRepository>(db);
    auto userRepo  = std::make_shared<UserRepository>(db);
    LoanService loanSvc(assetRepo, userRepo);
    NotificationService notifier(assetRepo, userRepo, {});

    // 2) Create asset + user + issue
    Asset a{"L1", AssetType::Laptop, "XPS", "Dell"};
    assetRepo->add(a);
    auto hash = hashPassword("pw");
    User u{"U1", "Alice", Role::User, hash};
    userRepo->add(u);
    ASSERT_TRUE(loanSvc.issueAsset("L1","U1"));

    // 3) Back‐date the loan to 15 days ago
    time_t fifteen_days_ago = std::time(nullptr) - 15*24*60*60;
    std::string sql =
      "UPDATE loans "
      "SET issue_date=" + std::to_string(fifteen_days_ago) + " "
      "WHERE asset_id='L1';";
    EXPECT_EQ(SQLITE_OK, sqlite3_exec(db->get(), sql.c_str(), nullptr, nullptr, nullptr));

    // 4) Now count overdue (default threshold = 14 days)
    int count = notifier.countOverdue();
    EXPECT_EQ(1, count);
}
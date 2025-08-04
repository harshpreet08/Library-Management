#include "CLI.h"
#include "../persistence/DatabaseManager.h"
#include "../persistence/AssetRepository.h"
#include "../persistence/UserRepository.h"
#include "../services/LoanService.h"
#include "../services/NotificationService.h"
#include "../services/EmailNotifier.h"
#include "../models/Asset.h"
#include "../models/User.h"
#include "Context.h"
#include <filesystem>
#include <iostream>
#include <memory>
#include <limits>
#include <string>
#include <algorithm>

static void printAssetHeader() {
    std::cout << "ID    | Type   | Title                  | Author/Owner         | Status    | Borrowed Info\n";
    std::cout << "---------------------------------------------------------------------------------------------\n";
}

static void printAssetRow(const std::string& id, const std::string& type,
                          const std::string& title, const std::string& authOwner,
                          const std::string& status, const std::string& extra) {
    auto pad = [](const std::string& s, size_t width) {
        if (s.size() >= width) return s.substr(0, width);
        return s + std::string(width - s.size(), ' ');
    };
    std::cout
        << pad(id, 5) << " | "
        << pad(type, 6) << " | "
        << pad(title, 22) << " | "
        << pad(authOwner, 20) << " | "
        << pad(status, 9) << " | "
        << extra << "\n";
}

static std::string normalizeCmd(const std::string& in) {
    std::string s = in;
    std::transform(s.begin(), s.end(), s.begin(), ::tolower);
    return s;
}

void CLI::printHelp() {
    std::cout << "\nCommands / shortcuts:\n"
              << "1 / a        : Add Asset (Book/Laptop)\n"
              << "2 / u        : Add User\n"
              << "3 / i        : Issue Asset to User\n"
              << "4 / r        : Return Asset\n"
              << "5 / l        : List All Assets\n"
              << "6 / o        : Show Overdues / Notify\n"
              << "7 / sa       : Search Asset by ID\n"
              << "8 / su       : Search User by ID\n"
              << "9 / lu       : List All Users\n"
              << "h / help     : Show this help\n"
              << "q / exit     : Quit\n";
}

void CLI::run() {
    const std::string context_file = "context.txt";
    std::string db_path = std::filesystem::current_path().string() + "/library.db";
    std::cout << "Welcome! Using database file: " << db_path << "\n";

    // load last context
    Context ctx = loadContext(context_file);
    if (!ctx.lastUser.empty() || !ctx.lastAsset.empty()) {
        std::cout << "Last used";
        if (!ctx.lastUser.empty()) std::cout << " user: " << ctx.lastUser;
        if (!ctx.lastAsset.empty()) std::cout << " asset: " << ctx.lastAsset;
        std::cout << "\n";
    }

    auto db = std::make_shared<DatabaseManager>(db_path);
    db->initializeSchema();

    auto assetRepo = std::make_shared<AssetRepository>(db);
    auto userRepo = std::make_shared<UserRepository>(db);
    LoanService loan(assetRepo, userRepo);

    std::vector<std::shared_ptr<NotificationStrategy>> strategies;
    strategies.push_back(std::make_shared<EmailNotifier>("noreply@library.local"));
    NotificationService notifier(assetRepo, userRepo, strategies);

    // Summary at startup
    int overdueCount = notifier.countOverdue();
    if (overdueCount > 0) {
        std::cout << "⚠️ You have " << overdueCount << " overdue "
                  << (overdueCount == 1 ? "asset" : "assets") << ". Use option 6 to view details.\n";
    } else {
        std::cout << "🎉 No overdue assets at the moment.\n";
    }

    while (true) {
        std::cout << "\nWhat would you like to do? (h for help)\n"
                  << "[1] Add Asset  [2] Add User  [3] Issue  [4] Return  [5] List  [6] Overdues  [7] Search Asset  [8] Search User  [9] List Users  [10] Exit\n"
                  << "Choice: ";
        std::string raw;
        if (!(std::cin >> raw)) break;
        std::string cmd = normalizeCmd(raw);

        if (cmd == "1" || cmd == "a") cmd = "add_asset";
        else if (cmd == "2" || cmd == "u") cmd = "add_user";
        else if (cmd == "3" || cmd == "i") cmd = "issue";
        else if (cmd == "4" || cmd == "r") cmd = "return";
        else if (cmd == "5" || cmd == "l") cmd = "list";
        else if (cmd == "6" || cmd == "o") cmd = "overdue";
        else if (cmd == "7" || cmd == "sa") cmd = "search_asset";
        else if (cmd == "8" || cmd == "su") cmd = "search_user";
        else if (cmd == "9" || cmd == "lu") cmd = "list_users";
        else if (cmd == "h" || cmd == "help") cmd = "help";
        else if (cmd == "10" || cmd == "q" || cmd == "exit") cmd = "exit";

        if (cmd == "help") {
            printHelp();
        } else if (cmd == "add_asset") {
            int subtype;
            std::cout << "Asset type: 1) Book 2) Laptop\nChoice: ";
            std::cin >> subtype;
            std::string id, title, owner;
            std::cout << "Asset ID: ";
            std::cin >> id;
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

            if (auto existing = assetRepo->find(id); existing.has_value()) {
                std::cout << "⚠️ Asset with ID \"" << id << "\" already exists. Try a different ID.\n";
                continue;
            }

            if (subtype == 1) {
                std::cout << "Title: ";
                std::getline(std::cin, title);
                std::cout << "Author: ";
                std::getline(std::cin, owner);
                Asset a(id, AssetType::Book, title, owner);
                assetRepo->add(a);
                std::cout << "✅ Added book \"" << title << "\" by " << owner << ".\n";
                ctx.lastAsset = id;
            } else if (subtype == 2) {
                std::cout << "Model/Name: ";
                std::getline(std::cin, title);
                std::cout << "Owner/Info: ";
                std::getline(std::cin, owner);
                Asset a(id, AssetType::Laptop, title, owner);
                assetRepo->add(a);
                std::cout << "✅ Added laptop \"" << title << "\" (info: " << owner << ").\n";
                ctx.lastAsset = id;
            } else {
                std::cout << "Invalid asset type selection.\n";
                continue;
            }
            saveContext(context_file, ctx);
        } else if (cmd == "add_user") {
            std::string id, name;
            std::cout << "User ID: ";
            std::cin >> id;
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cout << "Name: ";
            std::getline(std::cin, name);
            if (auto existing = userRepo->find(id); existing.has_value()) {
                std::cout << "⚠️ User with ID \"" << id << "\" already exists.\n";
            } else {
                User u(id, name);
                userRepo->add(u);
                std::cout << "✅ User \"" << name << "\" added.\n";
                ctx.lastUser = id;
                saveContext(context_file, ctx);
            }
        } else if (cmd == "issue") {
            std::string assetId, userId;
            std::cout << "Asset ID (or press enter to use last: " << ctx.lastAsset << "): ";
            std::cin >> assetId;
            if (assetId.empty() && !ctx.lastAsset.empty()) assetId = ctx.lastAsset;

            std::cout << "User ID (or press enter to use last: " << ctx.lastUser << "): ";
            std::cin >> userId;
            if (userId.empty() && !ctx.lastUser.empty()) userId = ctx.lastUser;

            if (loan.issueAsset(assetId, userId)) {
                ctx.lastAsset = assetId;
                ctx.lastUser = userId;
                saveContext(context_file, ctx);
            }
        } else if (cmd == "return") {
            std::string assetId;
            std::cout << "Asset ID (or press enter to use last: " << ctx.lastAsset << "): ";
            std::cin >> assetId;
            if (assetId.empty() && !ctx.lastAsset.empty()) assetId = ctx.lastAsset;

            // confirm destructive
            std::cout << "Are you sure you want to return asset \"" << assetId << "\"? (y/n): ";
            std::string yn;
            std::cin >> yn;
            if (!yn.empty() && (yn[0] == 'y' || yn[0] == 'Y')) {
                loan.returnAsset(assetId);
                ctx.lastAsset = assetId;
                saveContext(context_file, ctx);
            } else {
                std::cout << "Return cancelled.\n";
            }
        } else if (cmd == "list") {
            auto all = assetRepo->getAll();
            if (all.empty()) {
                std::cout << "No assets in the system yet.\n";
                continue;
            }
            printAssetHeader();
            for (auto& a : all) {
                std::string status = a.isIssued() ? "Issued" : "Available";
                std::string extra;
                if (a.isIssued()) {
                    if (auto loanInfo = loan.loanInfo(a.id()); loanInfo.has_value()) {
                        time_t now = std::time(nullptr);
                        double days = difftime(now, loanInfo->issueDate) / (60 * 60 * 24);
                        extra = "borrowed " + std::to_string(static_cast<int>(days)) + "d";
                        if (auto userOpt = userRepo->find(loanInfo->userId); userOpt.has_value()) {
                            extra += " by " + userOpt->name();
                        }
                    }
                }
                printAssetRow(a.id(), assetTypeToString(a.type()), a.title(),
                             a.authorOrOwner(), status, extra);
            }
        } else if (cmd == "overdue") {
            notifier.checkAndNotifyOverdue();
        } else if (cmd == "search_asset") {
            std::string assetId;
            std::cout << "Asset ID to search: ";
            std::cin >> assetId;
            if (auto assetOpt = assetRepo->find(assetId); assetOpt.has_value()) {
                auto& a = *assetOpt;
                std::cout << "Found: " << a.id() << " | " << assetTypeToString(a.type())
                          << " | " << a.title() << " | " << a.authorOrOwner()
                          << " | " << (a.isIssued() ? "Issued" : "Available") << "\n";
                ctx.lastAsset = assetId;
                saveContext(context_file, ctx);
            } else {
                std::cout << "Asset not found with ID \"" << assetId << "\".\n";
            }
        } else if (cmd == "search_user") {
            std::string userId;
            std::cout << "User ID to search: ";
            std::cin >> userId;
            if (auto userOpt = userRepo->find(userId); userOpt.has_value()) {
                std::cout << "Found user: " << userOpt->id() << " | " << userOpt->name() << "\n";
                ctx.lastUser = userId;
                saveContext(context_file, ctx);
            } else {
                std::cout << "User not found with ID \"" << userId << "\".\n";
            }
        } else if (cmd == "list_users") {
            auto allUsers = userRepo->getAll();
            if (allUsers.empty()) {
                std::cout << "No users registered yet.\n";
            } else {
                std::cout << "Users:\n";
                for (auto& u : allUsers) {
                    std::cout << " - " << u.id() << " | " << u.name() << "\n";
                }
            }
        } else if (cmd == "exit") {
            std::cout << "Thanks for using the system. Goodbye!\n";
            break;
        } else {
            std::cout << "Unknown command. Type 'h' or 'help' to see available commands.\n";
        }
    }
}
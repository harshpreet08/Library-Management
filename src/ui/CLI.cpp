#include "CLI.h"
#include "../persistence/DatabaseManager.h"
#include "../persistence/AssetRepository.h"
#include "../persistence/UserRepository.h"
#include "../services/LoanService.h"
#include "../services/NotificationService.h"
#include "../services/EmailNotifier.h"
#include "../models/Asset.h"
#include "../models/User.h"
#include "../util/Security.h"
#include "Context.h"

#include <filesystem>
#include <iostream>
#include <limits>
#include <memory>
#include <string>
#include <algorithm>
#include <vector>

// Globals for our repos & services
static std::shared_ptr<AssetRepository>     assetRepoPtr;
static std::shared_ptr<UserRepository>      userRepoPtr;
static std::unique_ptr<LoanService>         loanServicePtr;
static std::unique_ptr<NotificationService> notifierPtr;
static Context                              context;

// Pretty-print helpers
static void printAssetHeader() {
    std::cout
      << "ID    | Type   | Title                  | Author/Owner         | Status    | Borrowed Info\n"
      << "---------------------------------------------------------------------------------------------\n";
}
static void printAssetRow(const std::string& id,
                          const std::string& type,
                          const std::string& title,
                          const std::string& authOwner,
                          const std::string& status,
                          const std::string& extra)
{
    auto pad = [&](const std::string& s, size_t w){
        return s.size() >= w ? s.substr(0,w) : s + std::string(w-s.size(),' ');
    };
    std::cout
      << pad(id,5)         << " | "
      << pad(type,6)       << " | "
      << pad(title,22)     << " | "
      << pad(authOwner,20) << " | "
      << pad(status,9)     << " | "
      << extra             << "\n";
}

// Lowercase helper
static std::string normalize(const std::string& in) {
    std::string s = in;
    std::transform(s.begin(), s.end(), s.begin(), ::tolower);
    return s;
}

// Read a trimmed line
static std::string readLine() {
    std::string line;
    std::getline(std::cin, line);
    const char* ws = " \t\r\n";
    auto b = line.find_first_not_of(ws);
    if (b==std::string::npos) return "";
    auto e = line.find_last_not_of(ws);
    return line.substr(b, e-b+1);
}

// Print header for users
static void printUserHeader() {
    std::cout
      << "ID    | Name\n"
      << "--------------\n";
}

// Print user row
static void printUserRow(const std::string& id, const std::string& name) {
    auto pad = [&](const std::string& s, size_t w){
        return s.size() >= w ? s.substr(0, w) : s + std::string(w - s.size(), ' ');
    };
    std::cout
      << pad(id, 5) << " | "
      << name << "\n";
}

void CLI::printHelp() {
    std::cout << "\nCommands / shortcuts:\n"
              << "  1 / a     : Add Asset (Book/Laptop)\n"
              << "  2 / u     : Add User\n"
              << "  3 / i     : Issue Asset to User\n"
              << "  4 / r     : Return Asset\n"
              << "  5 / l     : List All Assets\n"
              << "  6 / o     : Show Overdues / Notify\n"
              << "  7 / sa    : Search Asset by ID\n"
              << "  8 / su    : Search User by ID\n"
              << "  9 / lu    : List All Users\n"
              << "  h / help  : Show this help\n"
              << "  q / exit  : Quit\n";
}

void CLI::run() {
    if (!initCrypto()) throw std::runtime_error("crypto init failed");
    const std::string ctxFile = "context.txt";
    std::string dbPath = std::filesystem::current_path().string() + "/library.db";
    std::cout << "Welcome! Using database: " << dbPath << "\n";
    context = loadContext(ctxFile);

    auto db = std::make_shared<DatabaseManager>(dbPath);
    db->initializeSchema();

    assetRepoPtr   = std::make_shared<AssetRepository>(db);
    userRepoPtr    = std::make_shared<UserRepository>(db);
    loanServicePtr = std::make_unique<LoanService>(assetRepoPtr, userRepoPtr);

    std::vector<std::shared_ptr<NotificationStrategy>> strat;
    strat.emplace_back(std::make_shared<EmailNotifier>("noreply@library.local"));
    notifierPtr = std::make_unique<NotificationService>(assetRepoPtr, userRepoPtr, strat);

    // Bootstrap staff
    if (userRepoPtr->getAll().empty()) {
        std::cout << "No users found. Create initial staff account.\n";
        std::string id, name, pwd;
        std::cout << "Staff ID: "; std::cin >> id;
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cout << "Name: "; std::getline(std::cin, name);
        std::cout << "Password: "; std::cin >> pwd;
        auto hash = hashPassword(pwd);
        userRepoPtr->add(User{id, name, Role::Staff, hash});
        std::cout << "✅ Staff \"" << name << "\" created.\n\n";
    }

    // Pre-login menu
    while (true) {
        std::cout << "\nMain Menu:\n"
                     " 1) Login\n"
                     " 2) Register\n"
                     " 3) Exit\n"
                     "Choice: ";
        std::string choice; std::cin >> choice;
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        choice = normalize(choice);
        if (choice=="1" || choice=="login") break;
        if (choice=="2" || choice=="register") {
            std::string id, name, pwd;
            std::cout<<"Choose user ID: "; std::cin>>id;
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cout<<"Your name: "; std::getline(std::cin,name);
            std::cout<<"Password: "; std::cin>>pwd;
            auto hash = hashPassword(pwd);
            userRepoPtr->add(User{id,name,Role::User,hash});
            std::cout<<"✅ Registered. Please login.\n";
            continue;
        }
        if (choice=="3"||choice=="exit"||choice=="q") return;
        std::cout<<"Invalid choice, try again.\n";
    }

    // Login
    User currentUser{"","",Role::User,""};
    while (true) {
        std::cout<<"User ID (or 'quit'): ";
        std::string uid = normalize(readLine());
        if (uid=="quit"||uid=="exit"||uid=="q") return;
        std::cout<<"Password: ";
        std::string pwd = readLine();

        if (auto opt = userRepoPtr->find(uid); opt.has_value()) {
            if (!verifyPassword(opt->passwordHash(), pwd)) {
                std::cout<<"❌ Invalid password.\n";
                continue;
            }
            currentUser = *opt;
            break;
        }
        std::cout<<"❌ User not found.\n";
    }

    // Role loop
    while (true) {
        if (currentUser.role()==Role::Staff) runStaffMenu(currentUser);
        else                                    runUserMenu(currentUser);

        std::cout<<"\nNext:\n"
                     " 1) Login another user\n"
                     " 2) Quit\n"
                     "Choice: ";
        std::string nxt; std::cin>>nxt;
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        nxt = normalize(nxt);
        if (nxt=="2"||nxt=="quit"||nxt=="exit"||nxt=="q") break;

        // re-login
        while (true) {
            std::cout<<"User ID (or 'quit'): ";
            std::string uid = normalize(readLine());
            if (uid=="quit"||uid=="exit"||uid=="q") { saveContext(ctxFile,context); return; }
            std::cout<<"Password: ";
            std::string pwd = readLine();
            if (auto opt = userRepoPtr->find(uid); opt.has_value()) {
                if (!verifyPassword(opt->passwordHash(), pwd)) {
                    std::cout<<"❌ Invalid password.\n"; continue;
                }
                currentUser = *opt;
                break;
            }
            std::cout<<"❌ User not found.\n";
        }
    }

    saveContext(ctxFile, context);
}

void CLI::runStaffMenu(const User& u) {
    std::cout<<"\n[Staff Menu] Welcome, "<<u.name()<<"!\n";
    int over = notifierPtr->countOverdue();
    if (over>0) std::cout<<"⚠️ You have "<<over<<" overdue assets.\n";
    else        std::cout<<"🎉 No overdue assets.\n";

    while (true) {
        std::cout<<"\n[Staff] (h=help) > ";
        std::string raw; std::cin>>raw;
        std::string cmd = normalize(raw);

        if      (cmd=="1"||cmd=="a")    cmd="add_asset";
        else if (cmd=="2"||cmd=="u")    cmd="add_user";
        else if (cmd=="3"||cmd=="i")    cmd="issue";
        else if (cmd=="4"||cmd=="r")    cmd="return";
        else if (cmd=="5"||cmd=="l")    cmd="list";
        else if (cmd=="6"||cmd=="o")    cmd="overdue";
        else if (cmd=="7"||cmd=="sa")   cmd="search_asset";
        else if (cmd=="8"||cmd=="su")   cmd="search_user";
        else if (cmd=="9"||cmd=="lu")   cmd="list_users";
        else if (cmd=="h")              cmd="help";
        else if (cmd=="10"||cmd=="q")   cmd="exit";

        if      (cmd=="help") {
            printHelp();
        }
        else if (cmd=="add_asset") {
            int subtype;
            std::cout<<"Asset type: 1) Book  2) Laptop\nChoice: "; std::cin>>subtype;
            std::string id;
            std::cout<<"Asset ID: "; std::cin>>id; std::cin.ignore(std::numeric_limits<std::streamsize>::max(),'\n');
            if (assetRepoPtr->find(id).has_value()) {
                std::cout<<"⚠️ Asset \""<<id<<"\" exists.\n";
                continue;
            }
            std::string title, owner;
            if (subtype==1) {
                std::cout<<"Title: "; std::getline(std::cin,title);
                std::cout<<"Author: "; std::getline(std::cin,owner);
                assetRepoPtr->add( Asset{id,AssetType::Book,title,owner} );
                std::cout<<"✅ Added book \""<<title<<"\" by "<<owner<<".\n";
            }
            else if (subtype==2) {
                std::cout<<"Model/Name: "; std::getline(std::cin,title);
                std::cout<<"Owner/Info: "; std::getline(std::cin,owner);
                assetRepoPtr->add( Asset{id,AssetType::Laptop,title,owner} );
                std::cout<<"✅ Added laptop \""<<title<<"\" (info: "<<owner<<").\n";
            }
            else {
                std::cout<<"Invalid type.\n";
            }
        }
        else if (cmd=="add_user") {
            std::string id,name,pwd;
            std::cout<<"User ID: "; std::cin>>id; std::cin.ignore(std::numeric_limits<std::streamsize>::max(),'\n');
            if (userRepoPtr->find(id).has_value()) {
                std::cout<<"⚠️ User \""<<id<<"\" exists.\n"; continue;
            }
            std::cout<<"Name: "; std::getline(std::cin,name);
            std::cout<<"Password: "; std::cin>>pwd;
            auto hash = hashPassword(pwd);
            userRepoPtr->add(User{id,name,Role::User,hash});
            std::cout<<"✅ Added user \""<<name<<"\".\n";
        }
        else if (cmd=="issue") {
            std::string aid,uid;
            std::cout<<"Asset ID: "; std::cin>>aid;
            std::cout<<"User ID: "; std::cin>>uid;
            if (loanServicePtr->issueAsset(aid,uid)) {
                std::cout<<"✅ "<<aid<<" → "<<uid<<"\n";
            }
        }
        else if (cmd=="return") {
            std::string aid; char c;
            std::cout<<"Asset ID: "; std::cin>>aid;
            std::cout<<"Confirm return? (y/n): "; std::cin>>c;
            if (c=='y'||c=='Y') {
                loanServicePtr->returnAsset(aid);
                std::cout<<"✅ "<<aid<<" returned.\n";
            } else std::cout<<"Cancelled.\n";
        }
        else if (cmd=="list") {
            auto all = assetRepoPtr->getAll();
            if (all.empty()) {
                std::cout<<"No assets.\n";
            } else {
                printAssetHeader();
                for (auto &a:all) {
                    std::string status = a.isIssued() ? "Issued":"Available";
                    std::string extra;
                    if (a.isIssued()) {
                        if (auto lo=loanServicePtr->loanInfo(a.id()); lo) {
                            int days=int((std::time(nullptr)-lo->issueDate)/(60*60*24));
                            extra = "borrowed "+std::to_string(days)+"d";
                            if (auto uu=userRepoPtr->find(lo->userId); uu)
                                extra += " by "+uu->name();
                        }
                    }
                    printAssetRow(
                        a.id(),
                        assetTypeToString(a.type()),
                        a.title(),
                        a.authorOrOwner(),
                        status,
                        extra
                    );
                }
            }
        }
        else if (cmd=="overdue") {
            notifierPtr->checkAndNotifyOverdue();
        }
        else if (cmd=="search_asset") {
            std::string aid;
            std::cout<<"Asset ID: "; std::cin>>aid;
            if (auto ao=assetRepoPtr->find(aid); ao) {
                auto &a=*ao;
                std::cout<<a.id()<<" | "<<assetTypeToString(a.type())
                         <<" | "<<a.title()<<" | "<<a.authorOrOwner()
                         <<" | "<<(a.isIssued()?"Issued":"Available")<<"\n";
            } else std::cout<<"Asset not found.\n";
        }
        else if (cmd=="search_user") {
            std::string uid;
            std::cout<<"User ID: "; std::cin>>uid;
            if (auto uo=userRepoPtr->find(uid); uo) {
                std::cout<<uo->id()<<" | "<<uo->name()<<"\n";
            } else std::cout<<"User not found.\n";
        }
        else if (cmd=="list_users") {
            auto users = userRepoPtr->getAll();
            if (users.empty()) {
                std::cout<<"No users in the system.\n";
            } else {
                printUserHeader();
                for (auto &usr:users)
                    printUserRow(usr.id(), usr.name());
            }
        }
        else if (cmd=="exit") {
            std::cout<<"Goodbye, "<<u.name()<<"!\n";
            break;
        }
        else {
            std::cout<<"Unknown command. 'h' for help.\n";
        }
    }
}

void CLI::runUserMenu(const User& u) {
    std::cout<<"\n[User Menu] Welcome, "<<u.name()<<"!\n";
    while (true) {
        std::cout<<"\n1) List Avail  2) Search  3) Issue  4) My Loans  5) Return  6) Exit\n> ";
        int c; if (!(std::cin>>c)) return;
        switch(c) {
            case 1: {
                printAssetHeader();
                for (auto &a:assetRepoPtr->getAll()) {
                    if (!a.isIssued())
                        printAssetRow(a.id(),assetTypeToString(a.type()),
                                     a.title(),a.authorOrOwner(),
                                     "Available","");
                }
                break;
            }
            case 2: {
                std::string aid;
                std::cout<<"Asset ID: "; std::cin>>aid;
                if (auto ao=assetRepoPtr->find(aid); ao) {
                    auto &a=*ao;
                    std::cout<<a.id()<<" | "<<assetTypeToString(a.type())
                             <<" | "<<a.title()<<" | "<<a.authorOrOwner()
                             <<" | "<<(a.isIssued()?"Issued":"Available")<<"\n";
                } else std::cout<<"Not found.\n";
                break;
            }
            case 3: {
                std::string aid;
                std::cout<<"Asset ID to borrow: "; std::cin>>aid;
                if (loanServicePtr->issueAsset(aid,u.id()))
                    std::cout<<"✅ Borrowed "<<aid<<"\n";
                break;
            }
            case 4: {
                printAssetHeader();
                for (auto &a:assetRepoPtr->getAll()) {
                    if (a.isIssued()) {
                        if (auto lo=loanServicePtr->loanInfo(a.id());
                            lo && lo->userId==u.id())
                        {
                            int days=int((std::time(nullptr)-lo->issueDate)/(60*60*24));
                            printAssetRow(a.id(),assetTypeToString(a.type()),
                                          a.title(),a.authorOrOwner(),
                                          "Issued",std::to_string(days)+"d ago");
                        }
                    }
                }
                break;
            }
            case 5: {
                std::string aid;
                std::cout<<"Asset ID to return: "; std::cin>>aid;
                std::cout<<"Confirm return? (y/n): ";
                char c2; std::cin>>c2;
                if (c2=='y'||c2=='Y') {
                    loanServicePtr->returnAsset(aid);
                    std::cout<<"✅ Returned "<<aid<<"\n";
                } else std::cout<<"Cancelled.\n";
                break;
            }
            case 6:
                std::cout<<"Goodbye, "<<u.name()<<"!\n";
                return;
            default:
                std::cout<<"Invalid choice.\n";
        }
    }
}
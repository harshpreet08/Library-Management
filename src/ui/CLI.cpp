#include "CLI.h"
#include "Context.h"
#include "../persistence/DatabaseManager.h"
#include "../persistence/AssetRepository.h"
#include "../persistence/UserRepository.h"
#include "../services/LoanService.h"
#include "../services/NotificationService.h"
#include "../services/EmailNotifier.h"
#include "../models/Asset.h"
#include "../models/User.h"
#include "../util/Security.h"

#include <filesystem>
#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <algorithm>

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

// User list helpers
static void printUserHeader() {
    std::cout
      << "ID    | Name\n"
      << "--------------\n";
}
static void printUserRow(const std::string& id, const std::string& name) {
    auto pad = [&](const std::string& s, size_t w){
        return s.size() >= w ? s.substr(0,w) : s + std::string(w - s.size(),' ');
    };
    std::cout
      << pad(id,5) << " | "
      << name << "\n";
}

// Helpers
static std::string normalize(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(), ::tolower);
    return s;
}
static std::string readLine() {
    std::string l; std::getline(std::cin,l);
    const char* ws=" \t\r\n";
    auto b=l.find_first_not_of(ws);
    if(b==std::string::npos) return "";
    auto e=l.find_last_not_of(ws);
    return l.substr(b,e-b+1);
}

void CLI::printHelp() {
    std::cout << "\nCommands / shortcuts:\n"
              << "  a / 1  : Add Asset (Book/Laptop)\n"
              << "  u / 2  : Add User\n"
              << "  i / 3  : Issue Asset\n"
              << "  r / 4  : Return Asset\n"
              << "  l / 5  : List Assets\n"
              << "  o / 6  : Show Overdues\n"
              << "  sa/7   : Search Asset\n"
              << "  su/8   : Search User\n"
              << "  lu/9   : List Users\n"
              << "  h      : Help\n"
              << "  q      : Quit\n";
}

void CLI::run() {
    if (!initCrypto()) throw std::runtime_error("crypto init failed");
    const std::string ctxFile = "context.txt";
    auto dbPath = std::filesystem::current_path() / "library.db";
    std::cout << "Welcome! Using database: " << dbPath.string() << "\n";
    context = loadContext(ctxFile);

    auto db = std::make_shared<DatabaseManager>(dbPath.string());
    db->initializeSchema();
    assetRepoPtr   = std::make_shared<AssetRepository>(db);
    userRepoPtr    = std::make_shared<UserRepository>(db);
    loanServicePtr = std::make_unique<LoanService>(assetRepoPtr, userRepoPtr);

    std::vector<std::shared_ptr<NotificationStrategy>> strategies;
    strategies.emplace_back(std::make_shared<EmailNotifier>("noreply@library.local"));
    notifierPtr = std::make_unique<NotificationService>(assetRepoPtr, userRepoPtr, strategies);

    // Bootstrap initial staff
    if (userRepoPtr->getAll().empty()) {
        std::cout << "No users found. Create initial staff account.\n";
        std::string id,name,pw;
        std::cout<<"Staff ID: "; std::cin>>id; std::cin.ignore();
        std::cout<<"Name: "; std::getline(std::cin,name);
        std::cout<<"Password: "; std::cin>>pw;
        userRepoPtr->add(User{id,name,Role::Staff,hashPassword(pw)});
        std::cout<<"✅ Staff \""<<name<<"\" created.\n\n";
    }

    // Pre-login menu
    while (true) {
        std::cout << "Main Menu:\n"
                  << " 1) Login\n"
                  << " 2) Register\n"
                  << " 3) Exit\n"
                  << "Choice: ";
        std::string c; std::cin>>c; std::cin.ignore();
        c = normalize(c);
        if (c=="1"||c=="login") break;
        if (c=="2"||c=="register") {
            std::string id,name,pw;
            std::cout<<"Choose ID: "; std::cin>>id; std::cin.ignore();
            std::cout<<"Name: "; std::getline(std::cin,name);
            std::cout<<"Password: "; std::cin>>pw;
            userRepoPtr->add(User{id,name,Role::User,hashPassword(pw)});
            std::cout<<"✅ Registered. Please login.\n";
            continue;
        }
        if (c=="3"||c=="exit"||c=="q") return;
        std::cout<<"Invalid choice.\n";
    }

    // Login flow
    User current{"","",Role::User,""};
    while (true) {
        std::cout<<"User ID["<<context.lastUser<<"]: ";
        auto uid = readLine();
        if (uid.empty()) uid=context.lastUser;
        if (uid=="q"||uid=="exit") return;
        std::cout<<"Password: "; auto pw=readLine();
        if (auto o=userRepoPtr->find(uid); o) {
            if (!verifyPassword(o->passwordHash(),pw)) {
                std::cout<<"Invalid password.\n"; continue;
            }
            current=*o;
            context.lastUser=uid;
            break;
        }
        std::cout<<"User not found.\n";
    }

    // Role loop
    while (true) {
        if (current.role()==Role::Staff) runStaffMenu(current);
        else                             runUserMenu(current);

        // After menu
        std::cout<<"\n1) Relaunch  2) Quit\nChoice: ";
        std::string nc; std::cin>>nc; std::cin.ignore();
        nc=normalize(nc);
        if (nc=="2"||nc=="quit") break;
    }

    saveContext(ctxFile,context);
}

void CLI::runStaffMenu(const User& u) {
    std::cout<<"\n[Staff] Welcome, "<<u.name()<<"!\n";
    int over=notifierPtr->countOverdue();
    std::cout<<(over? "⚠️ You have "+std::to_string(over)+" overdue assets.\n"
                   : "🎉 No overdue assets.\n");

    while (true) {
        std::cout<<"\n[Staff] (h=help) > ";
        std::string cmd; std::cin>>cmd; std::cin.ignore();
        cmd=normalize(cmd);
        if (cmd=="h"||cmd=="help")          { printHelp(); continue; }
        if (cmd=="1"||cmd=="a"||cmd=="add_asset") {
            int t; std::cout<<"Type 1)Book 2)Laptop: "; std::cin>>t; std::cin.ignore();
            std::string id; std::cout<<"Asset ID: "; std::cin>>id; std::cin.ignore();
            if (assetRepoPtr->find(id)) { std::cout<<"Exists.\n"; continue; }
            std::string title,owner;
            if (t==1) {
                std::cout<<"Title: "; std::getline(std::cin,title);
                std::cout<<"Author: "; std::getline(std::cin,owner);
                assetRepoPtr->add({id,AssetType::Book,title,owner});
                std::cout<<"Added book.\n";
            } else {
                std::cout<<"Model: "; std::getline(std::cin,title);
                std::cout<<"Info: ";  std::getline(std::cin,owner);
                assetRepoPtr->add({id,AssetType::Laptop,title,owner});
                std::cout<<"Added laptop.\n";
            }
            context.lastAsset=id;
        }
        else if (cmd=="2"||cmd=="u"||cmd=="add_user") {
            std::string id,name,pw;
            std::cout<<"User ID: "; std::cin>>id; std::cin.ignore();
            if (userRepoPtr->find(id)) { std::cout<<"Exists.\n"; continue; }
            std::cout<<"Name: "; std::getline(std::cin,name);
            std::cout<<"Password: "; std::cin>>pw;
            userRepoPtr->add({id,name,Role::User,hashPassword(pw)});
            std::cout<<"Added user.\n";
            context.lastUser=id;
        }
        else if (cmd=="3"||cmd=="i"||cmd=="issue") {
            std::string aid,uid;
            std::cout<<"Asset ID: "; std::cin>>aid;
            std::cout<<"User ID: "; std::cin>>uid;
            if (loanServicePtr->issueAsset(aid,uid)) {
                std::cout<<"Issued.\n";
                context.lastAsset=aid;
            }
        }
        else if (cmd=="4"||cmd=="r"||cmd=="return") {
            std::string aid; char c;
            std::cout<<"Asset ID: "; std::cin>>aid;
            std::cout<<"Confirm? (y/n): "; std::cin>>c;
            if (c=='y'||c=='Y') { loanServicePtr->returnAsset(aid); std::cout<<"Returned.\n"; }
        }
        else if (cmd=="5"||cmd=="l"||cmd=="list") {
            auto all=assetRepoPtr->getAll();
            if (all.empty()) { std::cout<<"No assets.\n"; continue; }
            printAssetHeader();
            for (auto &a:all) {
                std::string st=a.isIssued()?"Issued":"Available", extra;
                if (a.isIssued()) {
                    if (auto lo=loanServicePtr->loanInfo(a.id()); lo) {
                        int d=int((std::time(nullptr)-lo->issueDate)/86400);
                        extra="borrowed "+std::to_string(d)+"d by "+userRepoPtr->find(lo->userId)->name();
                    }
                }
                printAssetRow(a.id(),assetTypeToString(a.type()),a.title(),a.authorOrOwner(),st,extra);
            }
        }
        else if (cmd=="6"||cmd=="o"||cmd=="overdue") {
            notifierPtr->checkAndNotifyOverdue();
        }
        else if (cmd=="7"||cmd=="sa"||cmd=="search_asset") {
            std::string aid; std::cout<<"Asset ID: "; std::cin>>aid;
            if (auto ao=assetRepoPtr->find(aid)) {
                auto &a=*ao;
                std::cout<<a.id()<<" | "<<assetTypeToString(a.type())
                         <<" | "<<a.title()<<" | "<<a.authorOrOwner()
                         <<" | "<<(a.isIssued()?"Issued":"Available")<<"\n";
                context.lastAsset=aid;
            } else std::cout<<"Not found.\n";
        }
        else if (cmd=="8"||cmd=="su"||cmd=="search_user") {
            std::string uid; std::cout<<"User ID: "; std::cin>>uid;
            if (auto uo=userRepoPtr->find(uid)) {
                std::cout<<uo->id()<<" | "<<uo->name()<<"\n";
                context.lastUser=uid;
            } else std::cout<<"Not found.\n";
        }
        else if (cmd=="9"||cmd=="lu"||cmd=="list_users") {
            auto us=userRepoPtr->getAll();
            if (us.empty()) { std::cout<<"No users.\n"; continue; }
            printUserHeader();
            for (auto &u:us) printUserRow(u.id(),u.name());
        }
        else if (cmd=="q"||cmd=="exit") {
            std::cout<<"Goodbye, "<<u.name()<<"!\n";
            break;
        }
        else {
            std::cout<<"Unknown. 'h' for help.\n";
        }
    }
}

void CLI::runUserMenu(const User& u) {
    std::cout<<"\n[User] Welcome, "<<u.name()<<"!\n";
    while (true) {
        std::cout<<"\n1) List Avail 2) Search 3) Issue 4) My Loans 5) Return 6) Exit\n> ";
        int c; if (!(std::cin>>c)) return;
        switch(c) {
            case 1: {
                printAssetHeader();
                for (auto &a:assetRepoPtr->getAll())
                    if (!a.isIssued())
                        printAssetRow(a.id(),assetTypeToString(a.type()),a.title(),a.authorOrOwner(),"Available","");
                break;
            }
            case 2: {
                std::string aid; std::cout<<"Asset ID: "; std::cin>>aid;
                if (auto ao=assetRepoPtr->find(aid)) {
                    auto &a=*ao;
                    std::cout<<a.id()<<" | "<<assetTypeToString(a.type())
                             <<" | "<<a.title()<<" | "<<a.authorOrOwner()
                             <<" | "<<(a.isIssued()?"Issued":"Available")<<"\n";
                } else std::cout<<"Not found.\n";
                break;
            }
            case 3: {
                std::string aid; std::cout<<"Asset ID: "; std::cin>>aid;
                if (loanServicePtr->issueAsset(aid,u.id()))
                    std::cout<<"✅ Borrowed "<<aid<<"\n";
                break;
            }
            case 4: {
                printAssetHeader();
                for (auto &a:assetRepoPtr->getAll())
                    if (a.isIssued() && loanServicePtr->loanInfo(a.id())->userId==u.id()) {
                        int d=int((std::time(nullptr)-loanServicePtr->loanInfo(a.id())->issueDate)/86400);
                        printAssetRow(a.id(),assetTypeToString(a.type()),a.title(),a.authorOrOwner(),"Issued",std::to_string(d)+"d ago");
                    }
                break;
            }
            case 5: {
                std::string aid; char yn;
                std::cout<<"Asset ID: "; std::cin>>aid;
                std::cout<<"Confirm return? (y/n): "; std::cin>>yn;
                if (yn=='y'||yn=='Y') { loanServicePtr->returnAsset(aid); std::cout<<"Returned.\n"; }
                break;
            }
            case 6:
                std::cout<<"Goodbye, "<<u.name()<<"!\n";
                return;
            default:
                std::cout<<"Invalid.\n";
        }
    }
}
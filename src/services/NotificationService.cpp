#include "NotificationService.h"
#include "LoanService.h"
#include <iostream>
#include <sstream>

NotificationService::NotificationService(std::shared_ptr<AssetRepository> assetRepo,
                                         std::shared_ptr<UserRepository> userRepo,
                                         std::vector<std::shared_ptr<NotificationStrategy>> strategies)
    : _assetRepo(std::move(assetRepo)),
      _userRepo(std::move(userRepo)),
      _strategies(std::move(strategies)) {}

int NotificationService::countOverdue() {
    LoanService loan(_assetRepo, _userRepo);
    // capture overdue count without printing
    std::stringstream ss;
    auto originalBuf = std::cout.rdbuf(); // preserve if needed
    int count = 0;
    auto all = _assetRepo->getAll();
    time_t now = std::time(nullptr);
    for (auto& a : all) {
        if (!a.isIssued()) continue;
        auto loanInfoOpt = loan.loanInfo(a.id());
        if (!loanInfoOpt.has_value()) continue;
        double days = difftime(now, loanInfoOpt->issueDate) / (60 * 60 * 24);
        if (days > 14) {
            count++;
        }
    }
    return count;
}

void NotificationService::checkAndNotifyOverdue() {
    LoanService loan(_assetRepo, _userRepo);
    std::vector<std::string> overdueMessages;

    auto all = _assetRepo->getAll();
    time_t now = std::time(nullptr);
    for (auto& a : all) {
        if (!a.isIssued()) continue;
        auto loanInfoOpt = loan.loanInfo(a.id());
        if (!loanInfoOpt.has_value()) continue;
        double days = difftime(now, loanInfoOpt->issueDate) / (60 * 60 * 24);
        if (days > 14) {
            std::stringstream msg;
            msg << "OVERDUE: " << a.id()
                << " | " << assetTypeToString(a.type())
                << " | " << a.title()
                << " | borrowed " << static_cast<int>(days) << " days ago";
            if (auto userOpt = _userRepo->find(loanInfoOpt->userId); userOpt.has_value()) {
                msg << " by " << userOpt->name();
            }
            std::string formatted = msg.str();
            overdueMessages.push_back(formatted);
            // also print to console immediately
            std::cout << "⚠️ " << formatted << "\n";
        }
    }

    if (overdueMessages.empty()) {
        std::cout << "No overdue assets found.\n";
        return;
    }

    // Send summary via strategies
    std::stringstream summary;
    summary << "You have " << overdueMessages.size() << " overdue assets:\n";
    for (auto& m : overdueMessages) summary << "- " << m << "\n";

    std::string subject = "Overdue Assets Summary";
    std::string body = summary.str();

    // For demo, send to admin email (hardcoded)
    std::string recipient = "admin@library.local";
    for (auto& strategy : _strategies) {
        strategy->notify(recipient, subject, body);
    }
}
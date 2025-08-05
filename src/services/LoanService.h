#pragma once

#include "../persistence/AssetRepository.h"
#include "../persistence/UserRepository.h"
#include <memory>
#include <string>
#include <optional>

struct LoanInfo {
    std::string userId;
    time_t issueDate;
};

class LoanService {
public:
    LoanService(std::shared_ptr<AssetRepository> assetRepo, std::shared_ptr<UserRepository> userRepo);

    bool issueAsset(const std::string& assetId, const std::string& userId);
    bool returnAsset(const std::string& assetId);
    void listAll();
    void showOverdues(); // borrowed > 14 days

    // public accessor for outside consumers
    std::optional<LoanInfo> loanInfo(const std::string& assetId);

private:
    std::shared_ptr<AssetRepository> _assetRepo;
    std::shared_ptr<UserRepository> _userRepo;

    std::optional<LoanInfo> getLoanInfo(const std::string& assetId);
    void setLoan(const std::string& assetId, const std::string& userId);
    void clearLoan(const std::string& assetId);
};
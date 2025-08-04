#pragma once
#include <memory>
#include <vector>
#include "../persistence/AssetRepository.h"
#include "../persistence/UserRepository.h"
#include "NotificationStrategy.h"

class NotificationService {
public:
    NotificationService(std::shared_ptr<AssetRepository> assetRepo,
                        std::shared_ptr<UserRepository> userRepo,
                        std::vector<std::shared_ptr<NotificationStrategy>> strategies);

    void checkAndNotifyOverdue();
    int countOverdue();

private:
    std::shared_ptr<AssetRepository> _assetRepo;
    std::shared_ptr<UserRepository> _userRepo;
    std::vector<std::shared_ptr<NotificationStrategy>> _strategies;
};
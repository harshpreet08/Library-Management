#pragma once
#include <memory>
#include "../persistence/BookRepository.h"
#include "../persistence/UserRepository.h"

class NotificationService {
public:
    NotificationService(std::shared_ptr<BookRepository> bookRepo, std::shared_ptr<UserRepository> userRepo);
    void checkAndNotifyOverdue();

private:
    std::shared_ptr<BookRepository> _bookRepo;
    std::shared_ptr<UserRepository> _userRepo;
};
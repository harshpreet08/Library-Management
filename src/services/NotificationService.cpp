#include "NotificationService.h"
#include "LoanService.h"
#include <iostream>

NotificationService::NotificationService(std::shared_ptr<BookRepository> bookRepo, std::shared_ptr<UserRepository> userRepo)
    : _bookRepo(std::move(bookRepo)), _userRepo(std::move(userRepo)) {}

void NotificationService::checkAndNotifyOverdue() {
    LoanService loan(_bookRepo, _userRepo);
    loan.showOverdues();
}
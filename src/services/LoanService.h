#pragma once
#include "../persistence/BookRepository.h"
#include "../persistence/UserRepository.h"
#include <memory>
#include <string>

class LoanService {
public:
    LoanService(std::shared_ptr<BookRepository> repo, std::shared_ptr<UserRepository> userRepo);

    bool issueBook(const std::string& bookId, const std::string& userId);
    bool returnBook(const std::string& bookId);
    void listAll();
    void showOverdues(); // borrowed > 14 days

private:
    std::shared_ptr<BookRepository> _repo;
    std::shared_ptr<UserRepository> _userRepo;

    time_t getIssueDate(const std::string& bookId);
    void setLoan(const std::string& bookId, const std::string& userId);
    void clearLoan(const std::string& bookId);
};
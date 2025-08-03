#include "CLI.h"
#include "../persistence/DatabaseManager.h"
#include "../persistence/BookRepository.h"
#include "../persistence/UserRepository.h"
#include "../services/LoanService.h"
#include "../services/NotificationService.h"
#include "../models/Book.h"
#include "../models/User.h"
#include <filesystem>
#include <iostream>
#include <memory>
#include <limits>

void CLI::run() {
    std::string db_path = std::filesystem::current_path().string() + "/library.db";
    std::cout << "Welcome! Using database file: " << db_path << "\n";
    auto db = std::make_shared<DatabaseManager>(db_path);
    db->initializeSchema();

    auto bookRepo = std::make_shared<BookRepository>(db);
    auto userRepo = std::make_shared<UserRepository>(db);
    LoanService loan(bookRepo, userRepo);
    NotificationService notifier(bookRepo, userRepo);

    while (true) {
        std::cout << "\nWhat would you like to do?\n"
                  << "1. Add Book\n"
                  << "2. Add User\n"
                  << "3. Issue Book to User\n"
                  << "4. Return Book\n"
                  << "5. List All Books\n"
                  << "6. Show Overdues\n"
                  << "7. Exit\n"
                  << "Choice: ";

        int choice;
        if (!(std::cin >> choice)) {
            std::cout << "That wasn't a number. Please try again.\n";
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            continue;
        }

        switch (choice) {
            case 1: {
                std::string id, title, author;
                std::cout << "Book ID: ";
                std::cin >> id;
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                std::cout << "Title: ";
                std::getline(std::cin, title);
                std::cout << "Author: ";
                std::getline(std::cin, author);
                Book b(id, title, author);
                bookRepo->add(b);
                std::cout << "✅ Book \"" << title << "\" added successfully.\n";
                break;
            }
            case 2: {
                std::string id, name;
                std::cout << "User ID: ";
                std::cin >> id;
                std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                std::cout << "Name: ";
                std::getline(std::cin, name);
                User u(id, name);
                userRepo->add(u);
                std::cout << "✅ User \"" << name << "\" added successfully.\n";
                break;
            }
            case 3: {
                std::string bookId, userId;
                std::cout << "Book ID: ";
                std::cin >> bookId;
                std::cout << "User ID: ";
                std::cin >> userId;
                loan.issueBook(bookId, userId);
                break;
            }
            case 4: {
                std::string id;
                std::cout << "Book ID: ";
                std::cin >> id;
                loan.returnBook(id);
                break;
            }
            case 5:
                loan.listAll();
                break;
            case 6:
                notifier.checkAndNotifyOverdue();
                break;
            case 7:
                std::cout << "Thanks for using the system. Goodbye!\n";
                return;
            default:
                std::cout << "That's not a valid choice. Please pick a number from 1 to 7.\n";
        }
    }
}
#include "EmailNotifier.h"
#include <iostream>

EmailNotifier::EmailNotifier(std::string from)
    : _from(std::move(from)) {}

void EmailNotifier::notify(const std::string& recipient,
                           const std::string& subject,
                           const std::string& body) {
    // Stub: print formatted email to console
    std::cout << "\n[EMAIL STUB]\n"
              << "From: " << _from << "\n"
              << "To: " << recipient << "\n"
              << "Subject: " << subject << "\n\n"
              << body << "\n"
              << "---------------------------\n";
}
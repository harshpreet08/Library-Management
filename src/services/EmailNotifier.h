#pragma once
#include "NotificationStrategy.h"
#include <string>

class EmailNotifier : public NotificationStrategy {
public:
    explicit EmailNotifier(std::string from);
    void notify(const std::string& recipient,
                const std::string& subject,
                const std::string& body) override;

private:
    std::string _from;
};
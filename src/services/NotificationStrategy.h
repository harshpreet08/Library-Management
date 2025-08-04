#pragma once
#include <string>

class NotificationStrategy {
public:
    virtual ~NotificationStrategy() = default;
    virtual void notify(const std::string& recipient,
                        const std::string& subject,
                        const std::string& body) = 0;
};
#pragma once
#include <string>

class ILendable {
public:
    virtual ~ILendable() = default;
    virtual std::string id() const = 0;
    virtual bool isIssued() const = 0;
    virtual void setIssued(bool) = 0;
};
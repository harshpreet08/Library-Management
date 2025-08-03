#pragma once
#include "ILendable.h"
#include <string>

class Book : public ILendable {
public:
    Book(std::string id, std::string title, std::string author);

    std::string id() const override;
    bool isIssued() const override;
    void setIssued(bool issued) override;

    std::string title() const;
    std::string author() const;

private:
    std::string _id;
    std::string _title;
    std::string _author;
    bool _issued = false;
};
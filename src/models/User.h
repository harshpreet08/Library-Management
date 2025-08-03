#pragma once
#include <string>

class User {
public:
    User(std::string id, std::string name);
    std::string id() const;
    std::string name() const;

private:
    std::string _id;
    std::string _name;
};
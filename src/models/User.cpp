#include "User.h"

User::User(std::string id, std::string name)
    : _id(std::move(id)), _name(std::move(name)) {}

std::string User::id() const { return _id; }
std::string User::name() const { return _name; }
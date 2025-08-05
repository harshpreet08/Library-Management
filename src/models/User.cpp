#include "User.h"

User::User(std::string id, std::string name, Role role, std::string passwordHash)
    : _id(std::move(id))
    , _name(std::move(name))
    , _role(role)
    , _passwordHash(std::move(passwordHash))
{}

std::string User::id()           const { return _id; }
std::string User::name()         const { return _name; }
Role        User::role()         const { return _role; }
std::string User::passwordHash() const { return _passwordHash; }

void User::setRole(Role r)                    { _role = r; }
void User::setPasswordHash(const std::string& h) { _passwordHash = h; }
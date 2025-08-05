#pragma once
#include <string>

enum class Role { User, Staff };

inline Role    stringToRole(const std::string& s) { return s=="staff"?Role::Staff:Role::User; }
inline std::string roleToString(Role r)         { return r==Role::Staff?"staff":"user"; }

class User {
public:
    User(std::string id, std::string name, Role role, std::string passwordHash);

    std::string id()           const;
    std::string name()         const;
    Role        role()         const;
    std::string passwordHash() const;

    void setRole(Role r);
    void setPasswordHash(const std::string& h);

private:
    std::string _id;
    std::string _name;
    Role        _role;
    std::string _passwordHash;
};
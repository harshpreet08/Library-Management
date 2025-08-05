#pragma once
#include <string>

bool initCrypto();
std::string hashPassword(const std::string& pwd);
bool verifyPassword(const std::string& hash, const std::string& pwd);
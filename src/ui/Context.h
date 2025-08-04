#pragma once
#include <string>

struct Context {
    std::string lastUser;
    std::string lastAsset;
};

Context loadContext(const std::string& path);
void saveContext(const std::string& path, const Context& ctx);
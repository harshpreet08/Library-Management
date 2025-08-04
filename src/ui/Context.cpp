#include "Context.h"
#include <fstream>
#include <sstream>
#include <utility>

Context loadContext(const std::string& path) {
    Context ctx;
    std::ifstream in(path);
    if (!in) return ctx;
    std::string line;
    while (std::getline(in, line)) {
        if (line.rfind("lastUser=", 0) == 0) {
            ctx.lastUser = line.substr(std::string("lastUser=").size());
        } else if (line.rfind("lastAsset=", 0) == 0) {
            ctx.lastAsset = line.substr(std::string("lastAsset=").size());
        }
    }
    return ctx;
}

void saveContext(const std::string& path, const Context& ctx) {
    std::ofstream out(path, std::ios::trunc);
    if (!out) return;
    out << "lastUser=" << ctx.lastUser << "\n";
    out << "lastAsset=" << ctx.lastAsset << "\n";
}
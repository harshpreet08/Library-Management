#include "Context.h"
#include <fstream>
#include <sstream>

static constexpr char SEP = '\t';

// Splits “line” at the first SEP into left/right.
// If no SEP is found, left=line, right="".
static void split(const std::string& line,
                  std::string& left,
                  std::string& right)
{
    auto pos = line.find(SEP);
    if (pos == std::string::npos) {
        left  = line;
        right = "";
    } else {
        left  = line.substr(0, pos);
        right = line.substr(pos + 1);
    }
}

Context loadContext(const std::string& path)
{
    Context ctx;
    std::ifstream in(path);
    if (!in.is_open()) return ctx;  // missing → defaults

    std::string line;
    if (!std::getline(in, line)) return ctx;  // empty → defaults

    split(line, ctx.lastUser, ctx.lastAsset);
    return ctx;
}

void saveContext(const std::string& path, const Context& ctx)
{
    std::ofstream out(path, std::ios::trunc);
    // Write “user␉asset\n”
    out << ctx.lastUser << SEP << ctx.lastAsset << '\n';
}
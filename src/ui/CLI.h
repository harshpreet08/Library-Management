#pragma once

#include "../models/User.h"

class CLI {
public:
    void run();

private:
    void runStaffMenu(const User& u);
    void runUserMenu(const User& u);
    void printHelp();
};
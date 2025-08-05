# src/CMakeLists.txt
cmake_minimum_required(VERSION 3.25)
project(LibraryManagementCore LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(SQLite3 REQUIRED)

# — locate libsodium via pkg-config —
find_package(PkgConfig REQUIRED)
pkg_check_modules(SODIUM REQUIRED IMPORTED_TARGET libsodium)

add_library(core
        util/Security.h     util/Security.cpp

        models/User.h       models/User.cpp
        models/Asset.h      models/Asset.cpp

        persistence/DatabaseManager.h persistence/DatabaseManager.cpp
        persistence/UserRepository.h   persistence/UserRepository.cpp
        persistence/AssetRepository.h  persistence/AssetRepository.cpp

        services/LoanService.h        services/LoanService.cpp
        services/NotificationService.h services/NotificationService.cpp
        services/EmailNotifier.h      services/EmailNotifier.cpp

        ui/CLI.h      ui/CLI.cpp
        ui/Context.h  ui/Context.cpp
)

target_link_libraries(core PUBLIC
        ${SQLite3_LIBRARIES}
        PkgConfig::SODIUM
)

add_executable(app main.cpp)
target_link_libraries(app PRIVATE core)
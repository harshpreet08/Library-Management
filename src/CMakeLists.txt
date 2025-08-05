# src/CMakeLists.txt
cmake_minimum_required(VERSION 3.25)
project(LibraryManagementCore LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(SQLite3 REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(SODIUM REQUIRED IMPORTED_TARGET libsodium)
# — SMTP via libcurl —
find_package(CURL REQUIRED)

add_library(core
        util/Security.h     util/Security.cpp

        models/User.h       models/User.cpp
        models/Asset.h      models/Asset.cpp

        persistence/DatabaseManager.h persistence/DatabaseManager.cpp
        persistence/UserRepository.h   persistence/UserRepository.cpp
        persistence/AssetRepository.h  persistence/AssetRepository.cpp

        services/LoanService.h         services/LoanService.cpp
        services/NotificationService.h services/NotificationService.cpp
        services/EmailNotifier.h       services/EmailNotifier.cpp

        ui/CLI.h      ui/CLI.cpp
        ui/Context.h  ui/Context.cpp
)

target_link_libraries(core PUBLIC
        ${SQLite3_LIBRARIES}
        PkgConfig::SODIUM
        CURL::libcurl
)

# --- Application executable ---
add_executable(app main.cpp)
target_link_libraries(app PRIVATE core)

# --- GoogleTest integration ---
enable_testing()
include(FetchContent)

FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG        v1.14.0        # correct tag name
)
# On Windows this prevents overriding the parent's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

add_executable(tests
        tests/SecurityTests.cpp
        tests/UserRepositoryTests.cpp
        tests/AssetRepositoryTests.cpp
)
target_link_libraries(tests PRIVATE core gtest_main)
add_test(NAME AllTests COMMAND tests)
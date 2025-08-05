cmake_minimum_required(VERSION 3.25)
project(LibraryManagementCore LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# SQLite
find_package(SQLite3 REQUIRED)

# libsodium via pkg-config
find_package(PkgConfig REQUIRED)
pkg_check_modules(SODIUM REQUIRED IMPORTED_TARGET libsodium)

# —–– Fetch GoogleTest via FetchContent —–––––––––––––––––––––––––––––
include(FetchContent)
FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG        v1.14.0
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# —–– Core library —–––––––––––––––––––––––––––––––––––––––––––––––––––
add_library(core
        util/Security.h     util/Security.cpp
        models/User.h       models/User.cpp
        models/Asset.h      models/Asset.cpp

        persistence/DatabaseManager.h  persistence/DatabaseManager.cpp
        persistence/UserRepository.h   persistence/UserRepository.cpp
        persistence/AssetRepository.h  persistence/AssetRepository.cpp

        services/LoanService.h         services/LoanService.cpp
        services/NotificationService.h services/NotificationService.cpp
        services/EmailNotifier.h       services/EmailNotifier.cpp

        ui/CLI.h       ui/CLI.cpp
        ui/Context.h   ui/Context.cpp
)

# expose SQLite & Sodium to users of core
target_link_libraries(core PUBLIC
        ${SQLite3_LIBRARIES}
        PkgConfig::SODIUM
)
target_include_directories(core PUBLIC
        ${SQLite3_INCLUDE_DIRS}
        ${SODIUM_INCLUDE_DIRS}
)

# main executable
add_executable(app main.cpp)
target_link_libraries(app PRIVATE core)

# —–– Tests —––––––––––––––––––––––––––––––––––––––––––––––––––––––––––
enable_testing()

# pick up all *Tests.cpp under src/tests
file(GLOB TEST_SOURCES
        "${CMAKE_CURRENT_SOURCE_DIR}/tests/*Tests.cpp"
)

add_executable(tests ${TEST_SOURCES})
target_link_libraries(tests PRIVATE core gtest_main)
add_test(NAME AllTests COMMAND tests)
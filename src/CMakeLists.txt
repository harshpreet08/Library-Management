cmake_minimum_required(VERSION 3.25)
project(LibraryManagementCore LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(SQLite3 REQUIRED)

add_library(core
        models/ILendable.h

        models/Book.h models/Book.cpp            # can be deprecated later
        models/Asset.h models/Asset.cpp          # unified asset model
        models/User.h models/User.cpp

        persistence/DatabaseManager.h persistence/DatabaseManager.cpp
        persistence/AssetRepository.h persistence/AssetRepository.cpp
        persistence/UserRepository.h persistence/UserRepository.cpp

        services/LoanService.h services/LoanService.cpp
        services/NotificationService.h services/NotificationService.cpp

        ui/CLI.h ui/CLI.cpp
        services/NotificationStrategy.h
        services/EmailNotifier.cpp
        services/EmailNotifier.h
        ui/Context.cpp
        ui/Context.h
)

target_include_directories(core PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${SQLite3_INCLUDE_DIRS}
)

target_link_libraries(core PUBLIC
        ${SQLite3_LIBRARIES}
)

add_executable(app main.cpp
        services/NotificationStrategy.h
        services/EmailNotifier.cpp
        services/EmailNotifier.h
        ui/Context.cpp
        ui/Context.h)
target_link_libraries(app PRIVATE core)
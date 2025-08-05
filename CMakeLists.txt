# CMakeLists.txt (project root)
cmake_minimum_required(VERSION 3.25)
project(LibraryManagement LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(SQLite3 REQUIRED)

add_subdirectory(src)

enable_testing()
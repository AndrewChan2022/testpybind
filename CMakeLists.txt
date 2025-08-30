cmake_minimum_required(VERSION 3.20)
project(TestPyBind LANGUAGES CXX)

# Find pybind11
find_package(pybind11 REQUIRED)

# C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add src
add_subdirectory(src)

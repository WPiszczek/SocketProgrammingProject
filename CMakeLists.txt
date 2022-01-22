cmake_minimum_required(VERSION 3.10)

# set the project name
project(Hangman)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)

# add the executable
add_executable(Hangman Client.cpp globals.cpp server.cpp)

add_compile_options(-Wall -Wextra --std=c++20)

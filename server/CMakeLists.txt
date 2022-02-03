cmake_minimum_required(VERSION 3.10)

# set the project name
project(Hangman)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR})

# add the executable
add_executable(Hangman namespace.cpp Client.cpp HangmanGame.cpp server.cpp)
# add_executable(HangmanDawid serverDawid.cpp)

add_compile_options(-Wall -Wextra --std=c++20)

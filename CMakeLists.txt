cmake_minimum_required(VERSION 3.10)

# set the project name
project(Hangman)

# add the executable
add_executable(Hangman server.cpp)

add_compile_options(-Wall -Wextra)

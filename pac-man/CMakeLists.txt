cmake_minimum_required(VERSION 3.20)
project(pac-man C)

set(CMAKE_C_STANDARD 11)

add_subdirectory(thirdparty/raylib)

add_executable(pac-man src/main.c)

target_link_libraries(pac-man PRIVATE raylib)

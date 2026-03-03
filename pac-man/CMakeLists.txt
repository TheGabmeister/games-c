cmake_minimum_required(VERSION 3.20)
project(pacman C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
add_subdirectory(vendor/flecs)


file(GLOB_RECURSE MY_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/*.c")
add_executable(${PROJECT_NAME} 
    ${MY_SOURCES}
    vendor/cjson/cJSON.c
    vendor/tinyfiledialogs/tinyfiledialogs.c
    )

target_include_directories(${PROJECT_NAME} PRIVATE 
    src/ 
    vendor/nuklear
    vendor/cjson
    vendor/tinyfiledialogs
)
target_link_libraries(${PROJECT_NAME} PRIVATE
    flecs::flecs_static
)

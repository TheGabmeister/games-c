cmake_minimum_required(VERSION 3.20)
project(pacman C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
add_subdirectory(vendor/flecs)

set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
add_subdirectory(vendor/raylib)

set(BUILD_DEMOS OFF CACHE BOOL "" FORCE)
set(BUILD_SHARED OFF CACHE BOOL "" FORCE)
set(BUILD_STATIC ON CACHE BOOL "" FORCE)
add_subdirectory(vendor/Chipmunk2D)


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
    vendor/Chipmunk2D/include
)
target_link_libraries(${PROJECT_NAME} PRIVATE
    flecs::flecs_static
    raylib
    chipmunk_static
)

set(RES_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/res")
add_custom_target(copy_resources ALL
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${RES_SOURCE_DIR}"
            "$<TARGET_FILE_DIR:${PROJECT_NAME}>/res"
    COMMENT "Copying res folder next to executable"
)
add_dependencies(${PROJECT_NAME} copy_resources) # Force copy even when only building executable

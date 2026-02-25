cmake_minimum_required(VERSION 3.14)
project(space-invaders)

set(CMAKE_C_STANDARD 11)

include(FetchContent)
FetchContent_Declare(
    SDL3
    GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
    GIT_TAG release-3.4.2
)
FetchContent_Declare(
    flecs
    GIT_REPOSITORY https://github.com/SanderMertens/flecs.git
    GIT_TAG v4.1.4
)

FetchContent_MakeAvailable(SDL3 flecs)

add_executable(${PROJECT_NAME} src/main.c src/texture.c)
target_include_directories(${PROJECT_NAME} PRIVATE thirdparty)
target_link_libraries(${PROJECT_NAME} PRIVATE SDL3::SDL3 flecs::flecs_static)

# Copy SDL3 shared library next to the executable on Windows
if(WIN32)
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:SDL3::SDL3>
            $<TARGET_FILE_DIR:${PROJECT_NAME}>
    )
endif()

# Copy resources next to the executable
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/resources
        $<TARGET_FILE_DIR:${PROJECT_NAME}>/resources
)
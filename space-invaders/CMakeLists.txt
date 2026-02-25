cmake_minimum_required(VERSION 3.14)
project(space-invaders)

set(CMAKE_C_STANDARD 11)

include(FetchContent)
FetchContent_Declare(
    SDL3
    GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
    GIT_TAG release-3.4.2
)
FetchContent_MakeAvailable(SDL3)

add_executable(hello main.c)
target_link_libraries(hello PRIVATE SDL3::SDL3)

# Copy SDL3 shared library next to the executable on Windows
if(WIN32)
    add_custom_command(TARGET hello POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:SDL3::SDL3>
            $<TARGET_FILE_DIR:hello>
    )
endif()

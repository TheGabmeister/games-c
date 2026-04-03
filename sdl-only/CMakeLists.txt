cmake_minimum_required(VERSION 3.24)
project(sdl_only C)

set(CMAKE_C_STANDARD 17)

# SDL3 core
set(SDL_TEST_LIBRARY OFF CACHE BOOL "" FORCE)
add_subdirectory(vendor/sdl)


add_executable(${PROJECT_NAME}
    src/main.c
    src/platform.c
    src/input.c
    src/draw.c
    src/scene.c
    src/collectible.c
    src/game.c
    src/scenes/title_scene.c
    src/scenes/play_scene.c
)
target_link_libraries(${PROJECT_NAME} PRIVATE
    SDL3::SDL3
)

# Copy SDL3 shared library next to the executable on Windows
if(WIN32)
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:SDL3::SDL3>
            $<TARGET_FILE_DIR:${PROJECT_NAME}>
    )
endif()

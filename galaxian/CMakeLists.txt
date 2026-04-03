cmake_minimum_required(VERSION 3.24)
project(galaxian C)

set(CMAKE_C_STANDARD 17)

# SDL3 core
set(SDL_TEST_LIBRARY OFF CACHE BOOL "" FORCE)
add_subdirectory(vendor/sdl)

add_executable(${PROJECT_NAME}
    src/game.c
    src/platform.c
    src/game_state.c
    src/drawing.c
    src/galaxian.c
    src/particles.c
    src/state_menu.c
    src/state_gameplay.c
    src/state_gameover.c
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    SDL3::SDL3
)

if(WIN32)
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:SDL3::SDL3>
            $<TARGET_FILE_DIR:${PROJECT_NAME}>
    )
endif()

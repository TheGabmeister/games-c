cmake_minimum_required(VERSION 3.14)
project(space-invaders)

set(CMAKE_C_STANDARD 11)

add_subdirectory(thirdparty/sdl)
add_subdirectory(thirdparty/flecs)
add_subdirectory(thirdparty/cglm)

file(GLOB_RECURSE MY_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/*.c")
add_executable(${PROJECT_NAME} ${MY_SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE 
    src/ 
    thirdparty/stb 
    thirdparty/nuklear
    thirdparty/cjson
)
target_link_libraries(${PROJECT_NAME} PRIVATE 
    SDL3::SDL3 
    flecs::flecs_static
    cglm
)

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
        ${CMAKE_SOURCE_DIR}/assets
        $<TARGET_FILE_DIR:${PROJECT_NAME}>/assets
)
cmake_minimum_required(VERSION 3.24...3.30)
project(template)

# Generate compile_commands.json
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Dependencies
add_subdirectory(vendor/raylib)

set(ASSET_SOURCE_DIR ${CMAKE_SOURCE_DIR}/src/assets)

# Our Project
file(GLOB_RECURSE SOURCE_FILES CONFIGURE_DEPENDS src/*.c)
file(GLOB_RECURSE HEADER_FILES CONFIGURE_DEPENDS src/*.h)
add_executable(${PROJECT_NAME} ${SOURCE_FILES} ${HEADER_FILES})

set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${PROJECT_NAME})

set_property(TARGET ${PROJECT_NAME} PROPERTY VS_DEBUGGER_WORKING_DIRECTORY $<TARGET_FILE_DIR:${PROJECT_NAME}>)

if ("${PLATFORM}" STREQUAL "Web")
    add_custom_command(
        TARGET ${PROJECT_NAME} PRE_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${ASSET_SOURCE_DIR} $<TARGET_FILE_DIR:${PROJECT_NAME}>/../assets
    )
    #DEPENDS ${PROJECT_NAME}
else()
    add_custom_command(
        TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${ASSET_SOURCE_DIR} $<TARGET_FILE_DIR:${PROJECT_NAME}>/assets
    )
    #DEPENDS ${PROJECT_NAME}
endif()

# Strict warnings for our code only (not vendor)
if (MSVC)
    target_compile_definitions(${PROJECT_NAME} PRIVATE _CRT_SECURE_NO_WARNINGS)
    target_compile_options(${PROJECT_NAME} PRIVATE
        /W4             # High warning level
        /we4013         # Error: implicit function declaration (missing #include)
        /we4133         # Error: incompatible pointer types
        /we4715         # Error: not all paths return a value
    )
else()
    target_compile_options(${PROJECT_NAME} PRIVATE
        -Wall -Wextra
        -Werror=implicit-function-declaration
        -Werror=return-type
        -Werror=incompatible-pointer-types
    )
endif()

#set(raylib_VERBOSE 1)
target_link_libraries(${PROJECT_NAME} raylib)

# Web Configurations
if ("${PLATFORM}" STREQUAL "Web")
    # Tell Emscripten to build an example.html file.
    set_target_properties(${PROJECT_NAME} PROPERTIES SUFFIX ".html")
    target_link_options(${PROJECT_NAME} PUBLIC -sUSE_GLFW=3 PUBLIC --preload-file assets)
endif()

# Checks if OSX and links appropriate frameworks (Only required on MacOS)
if (APPLE)
    target_link_libraries(${PROJECT_NAME} "-framework IOKit")
    target_link_libraries(${PROJECT_NAME} "-framework Cocoa")
    target_link_libraries(${PROJECT_NAME} "-framework OpenGL")
endif()

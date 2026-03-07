cmake_minimum_required(VERSION 3.20)
project(pac-man C)

set(CMAKE_C_STANDARD 11)

add_subdirectory(thirdparty/raylib)

add_executable(pac-man
    src/main.c
    src/maze.c
    src/player.c
    src/ghost.c
    src/pellet.c
    src/score.c
    src/renderer.c
    src/audio.c
)

target_include_directories(pac-man PRIVATE src)
target_link_libraries(pac-man PRIVATE raylib)

if(MSVC) # If using the VS compiler...
    target_compile_definitions("${CMAKE_PROJECT_NAME}" PUBLIC _CRT_SECURE_NO_WARNINGS)
    set_target_properties("${CMAKE_PROJECT_NAME}" PROPERTIES LINK_FLAGS "/SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup") #no console
    
    #set_property(TARGET "${CMAKE_PROJECT_NAME}" PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreadedDebug<$<CONFIG:Debug>:Debug>")
    #set_property(TARGET "${CMAKE_PROJECT_NAME}" PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Release>:Release>")
endif()

#force remove unicode
if (WIN32)
    target_compile_options("${CMAKE_PROJECT_NAME}" PRIVATE /UUNICODE /U_UNICODE)
endif()
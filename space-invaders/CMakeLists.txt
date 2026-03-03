cmake_minimum_required(VERSION 3.14)
project(space-invaders)

set(CMAKE_C_STANDARD 11)

add_subdirectory(thirdparty/sdl)
add_subdirectory(thirdparty/flecs)
add_subdirectory(thirdparty/cglm)

# SDL_mixer: WAV, MP3 (dr_mp3), OGG Vorbis (stb_vorbis) — all header-only, no external deps
set(SDLMIXER_WAVE              ON)
set(SDLMIXER_MP3               ON)
set(SDLMIXER_MP3_DRMP3         ON)
set(SDLMIXER_MP3_MPG123        OFF)
set(SDLMIXER_VORBIS_STB        ON)
set(SDLMIXER_VORBIS_VORBISFILE OFF)
set(SDLMIXER_VORBIS_TREMOR     OFF)
set(SDLMIXER_FLAC              OFF)
set(SDLMIXER_GME               OFF)
set(SDLMIXER_MOD               OFF)
set(SDLMIXER_MIDI              OFF)
set(SDLMIXER_OPUS              OFF)
set(SDLMIXER_WAVPACK           OFF)
set(SDLMIXER_AIFF              OFF)
set(SDLMIXER_VOC               OFF)
set(SDLMIXER_AU                OFF)
add_subdirectory(thirdparty/sdl_mixer)

file(GLOB_RECURSE MY_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/*.c")
add_executable(${PROJECT_NAME} WIN32
    ${MY_SOURCES}
    thirdparty/cjson/cJSON.c)

target_include_directories(${PROJECT_NAME} PRIVATE 
    src/ 
    thirdparty/stb 
    thirdparty/nuklear
    thirdparty/cjson
)
target_link_libraries(${PROJECT_NAME} PRIVATE
    SDL3::SDL3
    SDL3_mixer::SDL3_mixer
    flecs::flecs_static
    cglm
)

# Copy SDL3 shared library next to the executable on Windows
if(WIN32)
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:SDL3::SDL3>
            $<TARGET_FILE:SDL3_mixer::SDL3_mixer>
            $<TARGET_FILE_DIR:${PROJECT_NAME}>
    )
endif()

# Copy resources next to the executable
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/assets
        $<TARGET_FILE_DIR:${PROJECT_NAME}>/assets
)
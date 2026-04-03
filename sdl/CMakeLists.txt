cmake_minimum_required(VERSION 3.24)
project(hello_sdl C)

set(CMAKE_C_STANDARD 17)

# SDL3 core
set(SDL_TEST_LIBRARY OFF CACHE BOOL "" FORCE)
add_subdirectory(vendor/sdl)

# SDL_image: JPEG (stb), PNG, BMP only
set(SDLIMAGE_VENDORED        ON  CACHE BOOL "" FORCE)
set(SDLIMAGE_BACKEND_STB     ON  CACHE BOOL "" FORCE)
set(SDLIMAGE_ANI             OFF CACHE BOOL "" FORCE)
set(SDLIMAGE_AVIF            OFF CACHE BOOL "" FORCE)
set(SDLIMAGE_BMP             ON  CACHE BOOL "" FORCE)
set(SDLIMAGE_GIF             OFF CACHE BOOL "" FORCE)
set(SDLIMAGE_JPG             ON  CACHE BOOL "" FORCE)
set(SDLIMAGE_JXL             OFF CACHE BOOL "" FORCE)
set(SDLIMAGE_LBM             OFF CACHE BOOL "" FORCE)
set(SDLIMAGE_PCX             OFF CACHE BOOL "" FORCE)
set(SDLIMAGE_PNG             ON  CACHE BOOL "" FORCE)
set(SDLIMAGE_PNM             OFF CACHE BOOL "" FORCE)
set(SDLIMAGE_QOI             OFF CACHE BOOL "" FORCE)
set(SDLIMAGE_SVG             OFF CACHE BOOL "" FORCE)
set(SDLIMAGE_TGA             OFF CACHE BOOL "" FORCE)
set(SDLIMAGE_TIF             OFF CACHE BOOL "" FORCE)
set(SDLIMAGE_WEBP            OFF CACHE BOOL "" FORCE)
set(SDLIMAGE_XCF             OFF CACHE BOOL "" FORCE)
set(SDLIMAGE_XPM             OFF CACHE BOOL "" FORCE)
set(SDLIMAGE_XV              OFF CACHE BOOL "" FORCE)
add_subdirectory(vendor/sdl_image)

# SDL_mixer: WAV, MP3 (dr_mp3), OGG Vorbis (stb_vorbis) — all header-only, no external deps
set(SDLMIXER_WAVE              ON  CACHE BOOL "" FORCE)
set(SDLMIXER_MP3               ON  CACHE BOOL "" FORCE)
set(SDLMIXER_MP3_DRMP3         ON  CACHE BOOL "" FORCE)
set(SDLMIXER_MP3_MPG123        OFF CACHE BOOL "" FORCE)
set(SDLMIXER_VORBIS_STB        ON  CACHE BOOL "" FORCE)
set(SDLMIXER_VORBIS_VORBISFILE OFF CACHE BOOL "" FORCE)
set(SDLMIXER_VORBIS_TREMOR     OFF CACHE BOOL "" FORCE)
set(SDLMIXER_FLAC              OFF CACHE BOOL "" FORCE)
set(SDLMIXER_GME               OFF CACHE BOOL "" FORCE)
set(SDLMIXER_MOD               OFF CACHE BOOL "" FORCE)
set(SDLMIXER_MIDI              OFF CACHE BOOL "" FORCE)
set(SDLMIXER_OPUS              OFF CACHE BOOL "" FORCE)
set(SDLMIXER_WAVPACK           OFF CACHE BOOL "" FORCE)
set(SDLMIXER_AIFF              OFF CACHE BOOL "" FORCE)
set(SDLMIXER_VOC               OFF CACHE BOOL "" FORCE)
set(SDLMIXER_AU                OFF CACHE BOOL "" FORCE)
add_subdirectory(vendor/sdl_mixer)

# SDL_ttf: TrueType only
set(SDLTTF_HARFBUZZ   OFF CACHE BOOL "" FORCE)
set(SDLTTF_PLUTOSVG   OFF CACHE BOOL "" FORCE)
set(SDLTTF_VENDORED   ON  CACHE BOOL "" FORCE)
add_subdirectory(vendor/sdl_ttf)

add_executable(hello_sdl
    src/main.c
    src/platform.c
    src/game_state.c
    src/resources.c
    src/drawing.c
    src/audio.c
)
target_link_libraries(hello_sdl PRIVATE
    SDL3::SDL3
    SDL3_image::SDL3_image
    SDL3_mixer::SDL3_mixer
    SDL3_ttf::SDL3_ttf
)

# Copy SDL3 shared library next to the executable on Windows
if(WIN32)
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:SDL3::SDL3>
            $<TARGET_FILE:SDL3_mixer::SDL3_mixer>
            $<TARGET_FILE:SDL3_image::SDL3_image>
            $<TARGET_FILE:SDL3_ttf::SDL3_ttf>
            $<TARGET_FILE_DIR:${PROJECT_NAME}>
    )
endif()
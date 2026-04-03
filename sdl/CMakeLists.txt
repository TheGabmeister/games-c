cmake_minimum_required(VERSION 3.24)
project(hello_sdl C)

set(CMAKE_C_STANDARD 17)

include(FetchContent)

FetchContent_Declare(
    SDL3
    URL https://github.com/libsdl-org/SDL/releases/download/release-3.4.2/SDL3-3.4.2.tar.gz
    FIND_PACKAGE_ARGS
)

# SDL_image: JPEG (stb), PNG, TIFF only
set(SDLIMAGE_VENDORED        ON)
set(SDLIMAGE_BACKEND_STB     ON)
set(SDLIMAGE_ANI             OFF)
set(SDLIMAGE_AVIF            OFF)
set(SDLIMAGE_BMP             ON)
set(SDLIMAGE_GIF             OFF)
set(SDLIMAGE_JPG             ON)
set(SDLIMAGE_JXL             OFF)
set(SDLIMAGE_LBM             OFF)
set(SDLIMAGE_PCX             OFF)
set(SDLIMAGE_PNG             ON)
set(SDLIMAGE_PNM             OFF)
set(SDLIMAGE_QOI             OFF)
set(SDLIMAGE_SVG             OFF)
set(SDLIMAGE_TGA             OFF)
set(SDLIMAGE_TIF             OFF)
set(SDLIMAGE_WEBP            OFF)
set(SDLIMAGE_XCF             OFF)
set(SDLIMAGE_XPM             OFF)
set(SDLIMAGE_XV              OFF)

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

# SDL_ttf: TrueType only
set(SDLTTF_HARFBUZZ   OFF CACHE BOOL "" FORCE)
set(SDLTTF_PLUTOSVG   OFF CACHE BOOL "" FORCE)
set(SDLTTF_VENDORED   ON  CACHE BOOL "" FORCE)

FetchContent_Declare(
    SDL3_image
    GIT_REPOSITORY https://github.com/libsdl-org/SDL_image.git
    GIT_TAG release-3.4.0
    GIT_SHALLOW TRUE
    GIT_SUBMODULES_RECURSE TRUE
    FIND_PACKAGE_ARGS
)

FetchContent_Declare(
    SDL3_mixer
    GIT_REPOSITORY https://github.com/libsdl-org/SDL_mixer.git
    GIT_TAG release-3.2.0
    GIT_SHALLOW TRUE
    GIT_SUBMODULES_RECURSE TRUE
    FIND_PACKAGE_ARGS
)

FetchContent_Declare(
    SDL3_ttf
    GIT_REPOSITORY https://github.com/libsdl-org/SDL_ttf.git
    GIT_TAG release-3.2.2
    GIT_SHALLOW TRUE
    GIT_SUBMODULES_RECURSE TRUE
    FIND_PACKAGE_ARGS
)

FetchContent_MakeAvailable(SDL3 SDL3_image SDL3_mixer SDL3_ttf)

add_executable(hello_sdl src/main.c)
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
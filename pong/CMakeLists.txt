cmake_minimum_required(VERSION 3.20)
project(pacman C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_subdirectory(vendor/sdl)
add_subdirectory(vendor/flecs)

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
add_subdirectory(vendor/sdl_mixer)

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
add_subdirectory(vendor/sdl_image)

# SDL_ttf: TrueType only
set(SDLTTF_HARFBUZZ   OFF CACHE BOOL "" FORCE)
set(SDLTTF_PLUTOSVG   OFF CACHE BOOL "" FORCE)
set(SDLTTF_VENDORED   ON  CACHE BOOL "" FORCE)
add_subdirectory(vendor/sdl_ttf)

file(GLOB_RECURSE MY_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/*.c")
add_executable(${PROJECT_NAME} 
    ${MY_SOURCES}
    )

target_include_directories(${PROJECT_NAME} PRIVATE
    vendor/nuklear
)
target_link_libraries(${PROJECT_NAME} PRIVATE
    SDL3::SDL3
    SDL3_mixer::SDL3_mixer
    SDL3_image::SDL3_image
    SDL3_ttf::SDL3_ttf
    flecs::flecs_static
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

set(RES_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/assets")
add_custom_target(copy_resources
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${RES_SOURCE_DIR}"
            "$<TARGET_FILE_DIR:${PROJECT_NAME}>/assets"
    COMMENT "Copying res folder next to executable"
)
add_dependencies(${PROJECT_NAME} copy_resources) # Force copy even when only building executable

add_library(engine SHARED
    engine.c
)

target_include_directories(engine
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(engine
    PRIVATE
        SDL3::SDL3
)

target_compile_definitions(engine
    PRIVATE
        BUILD_LIBTYPE_SHARED
    INTERFACE
        USE_LIBTYPE_SHARED
)

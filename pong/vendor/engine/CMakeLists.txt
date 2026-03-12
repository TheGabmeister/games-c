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
        ENGINE_BUILD_SHARED
    INTERFACE
        ENGINE_USE_SHARED
)

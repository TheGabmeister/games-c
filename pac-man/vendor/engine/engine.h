#ifndef ENGINE_H
#define ENGINE_H

#include <stdbool.h>
#include <SDL3/SDL.h>

// Function specifiers in case library is build/used as a shared library
// NOTE: Microsoft specifiers to tell compiler that symbols are imported/exported from a .dll
// NOTE: visibility("default") attribute makes symbols "visible" when compiled with -fvisibility=hidden
#if defined(_WIN32)
    #if defined(__TINYC__)
        #define __declspec(x) __attribute__((x))
    #endif
    #if defined(ENGINE_BUILD_SHARED)
        #define ENGINE_API __declspec(dllexport)     // We are building the library as a Win32 shared library (.dll)
    #elif defined(ENGINE_USE_SHARED)
        #define ENGINE_API __declspec(dllimport)     // We are using the library as a Win32 shared library (.dll)
    #endif
#else
    #if defined(ENGINE_BUILD_SHARED)
        #define ENGINE_API __attribute__((visibility("default"))) // We are building as a Unix shared library (.so/.dylib)
    #endif
#endif

#ifndef ENGINE_API
    #define ENGINE_API       // Functions defined as 'extern' by default (implicit specifiers)
#endif

ENGINE_API void init_window(int width, int height, const char *title);  // Initialize window and OpenGL context
ENGINE_API void close_window(void);                                     // Close window and unload OpenGL context
ENGINE_API bool is_window_ready(void);
ENGINE_API int get_fps(void);                                           // Return latest FPS computed from delta time
ENGINE_API float get_deltatime(void);                                   // Compute and return seconds since last frame
ENGINE_API SDL_Renderer *get_renderer(void);                            // Return the SDL renderer

#endif // ENGINE_H


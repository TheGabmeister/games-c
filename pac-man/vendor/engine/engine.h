#ifndef ENGINE_H
#define ENIGNE_H

// Function specifiers in case library is build/used as a shared library
// NOTE: Microsoft specifiers to tell compiler that symbols are imported/exported from a .dll
// NOTE: visibility("default") attribute makes symbols "visible" when compiled with -fvisibility=hidden
#if defined(_WIN32)
    #if defined(__TINYC__)
        #define __declspec(x) __attribute__((x))
    #endif
    #if defined(BUILD_LIBTYPE_SHARED)
        #define ENGINE_API __declspec(dllexport)     // We are building the library as a Win32 shared library (.dll)
    #elif defined(USE_LIBTYPE_SHARED)
        #define ENGINE_API __declspec(dllimport)     // We are using the library as a Win32 shared library (.dll)
    #endif
#else
    #if defined(BUILD_LIBTYPE_SHARED)
        #define ENGINE_API __attribute__((visibility("default"))) // We are building as a Unix shared library (.so/.dylib)
    #endif
#endif

#ifndef ENGINE_API
    #define ENGINE_API       // Functions defined as 'extern' by default (implicit specifiers)
#endif

ENGINE_API void init_window(int width, int height, const char *title);  // Initialize window and OpenGL context
ENGINE_API void close_window(void);                                     // Close window and unload OpenGL context
ENGINE_API bool is_window_ready(void); 

#endif // ENGINE_H


#ifndef PLATFORM_H
#define PLATFORM_H

#include <stdbool.h>
#include <SDL3/SDL.h>

// Function specifiers in case library is build/used as a shared library
// NOTE: Microsoft specifiers to tell compiler that symbols are imported/exported from a .dll
// NOTE: visibility("default") attribute makes symbols "visible" when compiled with -fvisibility=hidden
#if defined(_WIN32)
    #if defined(__TINYC__)
        #define __declspec(x) __attribute__((x))
    #endif
    #if defined(PLATFORM_BUILD_SHARED)
        #define PLATFORM_API __declspec(dllexport)     // We are building the library as a Win32 shared library (.dll)
    #elif defined(PLATFORM_USE_SHARED)
        #define PLATFORM_API __declspec(dllimport)     // We are using the library as a Win32 shared library (.dll)
    #endif
#else
    #if defined(PLATFORM_BUILD_SHARED)
        #define PLATFORM_API __attribute__((visibility("default"))) // We are building as a Unix shared library (.so/.dylib)
    #endif
#endif

#ifndef PLATFORM_API
    #define PLATFORM_API       // Functions defined as 'extern' by default (implicit specifiers)
#endif

typedef struct { float x, y; } vector2;
typedef struct { float x, y, z; } vector3;
typedef struct { float x, y, z, w; } vector4;

// 2D camera
typedef struct camera {
    vector2 offset;         
    vector2 target;         
    float rotation;         
    float zoom;             
} camera;

bool init_window(int width, int height, const char *title);
void close_window(void);                                     
bool is_window_ready(void);
float get_delta_time(void);
int get_fps(void);                                                                          
SDL_Renderer *get_renderer(void);                            


#endif // PLATFORM_H


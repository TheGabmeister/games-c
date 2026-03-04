#include "engine.h"
#include <SDL3/SDL.h>

// Core global state context data
typedef struct Globals {
    
    const char *title;                  // Window text title const pointer
    bool ready;                         // Check if window has been initialized successfully

    SDL_Window   *window;
    SDL_Renderer *renderer;

} Globals;

Globals GLOBALS = { 0 };   

void init_window(int width, int height, const char *title)
{
    GLOBALS.window   = NULL;
    GLOBALS.renderer = NULL;

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_Log("SDL_Init failed: %s", SDL_GetError());
        return;
    }

    GLOBALS.window = SDL_CreateWindow(title, width, height, 0);
    if (!GLOBALS.window) {
        SDL_Log("SDL_CreateWindow failed: %s", SDL_GetError());
        SDL_Quit();
        return;
    }

    GLOBALS.renderer = SDL_CreateRenderer(GLOBALS.window, NULL);
    if (!GLOBALS.renderer) {
        SDL_Log("SDL_CreateRenderer failed: %s", SDL_GetError());
        SDL_DestroyWindow(GLOBALS.window);
        SDL_Quit();
        return;
    }

    GLOBALS.ready = true;
}

void close_window(void)
{
    SDL_DestroyRenderer(GLOBALS.renderer);
    SDL_DestroyWindow(GLOBALS.window);
    SDL_Quit();
}

bool is_window_ready(void)
{
    return GLOBALS.ready;
}

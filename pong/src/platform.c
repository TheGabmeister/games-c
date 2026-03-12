#include "platform.h"
#include <SDL3/SDL.h>

// Core global state context data
typedef struct Globals {

    SDL_Window   *window;
    SDL_Renderer *renderer;
    bool ready;                         // Check if window has been initialized successfully

} Globals;

Globals GLOBALS = { 0 };   

void init_window(int width, int height, const char *title)
{
    GLOBALS.window   = NULL;
    GLOBALS.renderer = NULL;

    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO)) {
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
}

void close_window(void)
{
    if (GLOBALS.renderer) {
        SDL_DestroyRenderer(GLOBALS.renderer);
    }
    if (GLOBALS.window) {
        SDL_DestroyWindow(GLOBALS.window);
    }

    GLOBALS.renderer = NULL;
    GLOBALS.window = NULL;

    SDL_Quit();
}

float get_delta_time(void)
{
    static Uint64 previous_counter = 0;
    const Uint64 current_counter = SDL_GetPerformanceCounter();
    const Uint64 frequency = SDL_GetPerformanceFrequency();

    if (frequency == 0) {
        return 0.0f;
    }

    if (previous_counter == 0) {
        previous_counter = current_counter;
        return 0.0f;
    }

    const Uint64 elapsed = current_counter - previous_counter;
    previous_counter = current_counter;

    return (float)((double)elapsed / (double)frequency);
}

int get_fps(void)
{
    return 0;
}

SDL_Renderer *get_renderer(void)
{
    return GLOBALS.renderer;
}

bool is_window_ready(void)
{
    return GLOBALS.ready;
}

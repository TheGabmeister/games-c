#include "platform.h"
#include <SDL3/SDL.h>

// Core global state context data
typedef struct Globals {

    SDL_Window   *window;
    SDL_Renderer *renderer;
    bool ready;   
    Uint64 previous_ticks_ns;
    float delta_time;
    int fps;                      // Check if window has been initialized successfully

} Globals;

static Globals GLOBALS = { 0 };

static void _reset_globals(void)
{
    GLOBALS = (Globals){0};
}

bool init_window(int width, int height, const char *title)
{
    _reset_globals();

    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO)) {
        SDL_Log("SDL_Init failed: %s", SDL_GetError());
        return false;
    }

    GLOBALS.window = SDL_CreateWindow(title, width, height, 0);
    if (!GLOBALS.window) {
        SDL_Log("SDL_CreateWindow failed: %s", SDL_GetError());
        close_window();
        return false;
    }

    GLOBALS.renderer = SDL_CreateRenderer(GLOBALS.window, NULL);
    if (!GLOBALS.renderer) {
        SDL_Log("SDL_CreateRenderer failed: %s", SDL_GetError());
        close_window();
        return false;
    }

    GLOBALS.ready = true;
    GLOBALS.previous_ticks_ns = SDL_GetTicksNS();
    return true;
}

void close_window(void)
{
    if (GLOBALS.renderer) {
        SDL_DestroyRenderer(GLOBALS.renderer);
    }
    if (GLOBALS.window) {
        SDL_DestroyWindow(GLOBALS.window);
    }

    _reset_globals();
    SDL_Quit();
}

float get_delta_time(void)
{
    Uint64 current_ticks_ns;
    Uint64 elapsed_ticks_ns;

    if (!GLOBALS.ready) {
        return 0.0f;
    }

    current_ticks_ns = SDL_GetTicksNS();
    if (GLOBALS.previous_ticks_ns == 0) {
        GLOBALS.previous_ticks_ns = current_ticks_ns;
        GLOBALS.delta_time = 0.0f;
        GLOBALS.fps = 0;
        return GLOBALS.delta_time;
    }

    elapsed_ticks_ns = current_ticks_ns - GLOBALS.previous_ticks_ns;
    GLOBALS.previous_ticks_ns = current_ticks_ns;
    GLOBALS.delta_time = (float)elapsed_ticks_ns / 1e9f;
    GLOBALS.fps = (GLOBALS.delta_time > 0.0f) ? (int)(1.0f / GLOBALS.delta_time) : 0;

    return GLOBALS.delta_time;
}

int get_fps(void)
{
    return GLOBALS.fps;
}

SDL_Renderer *get_renderer(void)
{
    return GLOBALS.renderer;
}

bool is_window_ready(void)
{
    return GLOBALS.ready;
}

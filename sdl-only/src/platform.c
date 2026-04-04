#include "platform.h"
#include <SDL3/SDL.h>

// Core global state context data
typedef struct Globals {

    const char *title;
    bool ready;
    bool should_close;
    int width;
    int height;

    SDL_Window   *window;
    SDL_Renderer *renderer;
    Uint64 previous_ticks_ns;
    float delta_time;
    float fps_smoothed;

} Globals;

Globals GLOBALS = { 0 };

void init_window(int width, int height, const char *title)
{
    GLOBALS.window   = NULL;
    GLOBALS.renderer = NULL;
    GLOBALS.previous_ticks_ns = 0;
    GLOBALS.delta_time = 0.0f;
    GLOBALS.fps_smoothed = 0.0f;
    GLOBALS.width = width;
    GLOBALS.height = height;

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
    GLOBALS.previous_ticks_ns = SDL_GetTicksNS();
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
    GLOBALS.ready = false;
    GLOBALS.previous_ticks_ns = 0;
    GLOBALS.delta_time = 0.0f;
    GLOBALS.fps_smoothed = 0.0f;

    SDL_Quit();
}

bool is_window_ready(void)
{
    return GLOBALS.ready;
}

void request_close(void)
{
    GLOBALS.should_close = true;
}

int get_window_width(void)
{
    return GLOBALS.width;
}

int get_window_height(void)
{
    return GLOBALS.height;
}

#define MAX_DT (1.0f / 15.0f)

SDL_Renderer *get_renderer(void)
{
    return GLOBALS.renderer;
}

void platform_update_timing(void)
{
    Uint64 current_ticks_ns;
    Uint64 elapsed_ticks_ns;

    if (!GLOBALS.ready)
        return;

    current_ticks_ns = SDL_GetTicksNS();
    if (GLOBALS.previous_ticks_ns == 0) {
        GLOBALS.previous_ticks_ns = current_ticks_ns;
        GLOBALS.delta_time = 0.0f;
        GLOBALS.fps_smoothed = 0.0f;
        return;
    }

    elapsed_ticks_ns = current_ticks_ns - GLOBALS.previous_ticks_ns;
    GLOBALS.previous_ticks_ns = current_ticks_ns;

    float raw_dt = (float)elapsed_ticks_ns / 1e9f;
    GLOBALS.delta_time = (raw_dt > MAX_DT) ? MAX_DT : raw_dt;

    if (GLOBALS.delta_time > 0.0f) {
        float instant_fps = 1.0f / GLOBALS.delta_time;
        GLOBALS.fps_smoothed = (GLOBALS.fps_smoothed == 0.0f)
            ? instant_fps
            : 0.95f * GLOBALS.fps_smoothed + 0.05f * instant_fps;
    }
}

int get_fps(void)
{
    return (int)(GLOBALS.fps_smoothed + 0.5f);
}

float get_deltatime(void)
{
    return GLOBALS.delta_time;
}


void platform_process_event(SDL_Event *event)
{
    if (event->type == SDL_EVENT_QUIT)
        GLOBALS.should_close = true;
}

bool window_should_close(void)
{
    return GLOBALS.should_close;
}


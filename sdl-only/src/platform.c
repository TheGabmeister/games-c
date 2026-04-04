#include "platform.h"
#include <SDL3/SDL.h>

typedef struct {
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
} Platform;

static Platform platform = { 0 };

void init_window(int width, int height, const char *title)
{
    platform.window   = NULL;
    platform.renderer = NULL;
    platform.previous_ticks_ns = 0;
    platform.delta_time = 0.0f;
    platform.fps_smoothed = 0.0f;
    platform.width = width;
    platform.height = height;

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_Log("SDL_Init failed: %s", SDL_GetError());
        return;
    }

    platform.window = SDL_CreateWindow(title, width, height, 0);
    if (!platform.window) {
        SDL_Log("SDL_CreateWindow failed: %s", SDL_GetError());
        SDL_Quit();
        return;
    }

    platform.renderer = SDL_CreateRenderer(platform.window, NULL);
    if (!platform.renderer) {
        SDL_Log("SDL_CreateRenderer failed: %s", SDL_GetError());
        SDL_DestroyWindow(platform.window);
        SDL_Quit();
        return;
    }

    platform.ready = true;
    platform.previous_ticks_ns = SDL_GetTicksNS();
}

void close_window(void)
{
    if (platform.renderer) {
        SDL_DestroyRenderer(platform.renderer);
    }
    if (platform.window) {
        SDL_DestroyWindow(platform.window);
    }

    platform.renderer = NULL;
    platform.window = NULL;
    platform.ready = false;
    platform.previous_ticks_ns = 0;
    platform.delta_time = 0.0f;
    platform.fps_smoothed = 0.0f;

    SDL_Quit();
}

bool is_window_ready(void)
{
    return platform.ready;
}

void request_close(void)
{
    platform.should_close = true;
}

int get_window_width(void)
{
    return platform.width;
}

int get_window_height(void)
{
    return platform.height;
}

#define MAX_DT (1.0f / 15.0f)

SDL_Renderer *get_renderer(void)
{
    return platform.renderer;
}

void platform_update_timing(void)
{
    Uint64 current_ticks_ns;
    Uint64 elapsed_ticks_ns;

    if (!platform.ready)
        return;

    current_ticks_ns = SDL_GetTicksNS();
    if (platform.previous_ticks_ns == 0) {
        platform.previous_ticks_ns = current_ticks_ns;
        platform.delta_time = 0.0f;
        platform.fps_smoothed = 0.0f;
        return;
    }

    elapsed_ticks_ns = current_ticks_ns - platform.previous_ticks_ns;
    platform.previous_ticks_ns = current_ticks_ns;

    float raw_dt = (float)elapsed_ticks_ns / 1e9f;
    platform.delta_time = (raw_dt > MAX_DT) ? MAX_DT : raw_dt;

    if (platform.delta_time > 0.0f) {
        float instant_fps = 1.0f / platform.delta_time;
        platform.fps_smoothed = (platform.fps_smoothed == 0.0f)
            ? instant_fps
            : 0.95f * platform.fps_smoothed + 0.05f * instant_fps;
    }
}

int get_fps(void)
{
    return (int)(platform.fps_smoothed + 0.5f);
}

float get_deltatime(void)
{
    return platform.delta_time;
}


void platform_process_event(SDL_Event *event)
{
    if (event->type == SDL_EVENT_QUIT)
        platform.should_close = true;
}

bool window_should_close(void)
{
    return platform.should_close;
}


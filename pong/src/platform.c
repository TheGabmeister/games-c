#include "platform.h"
#include <SDL3/SDL.h>

// Core global state context data
typedef struct Globals {

    const char *title;                  // Window text title const pointer
    bool ready;                         // Check if window has been initialized successfully
    bool should_close;                  // Set when SDL_EVENT_QUIT is received

    SDL_Window   *window;
    SDL_Renderer *renderer;

    struct {
        bool currentKeyState[SDL_SCANCODE_COUNT];
        bool previousKeyState[SDL_SCANCODE_COUNT];
    } Keyboard;

    struct {
        SDL_MouseButtonFlags currentButtons;
        SDL_MouseButtonFlags previousButtons;
        float x, y;
        float wheelY;
    } Mouse;

} Globals;

Globals GLOBALS = { 0 };   
static Uint64 PREVIOUS_TICKS_NS = 0;
static float DELTA_TIME_SECONDS = 0.0f;
static int FPS = 0;

void init_window(int width, int height, const char *title)
{
    GLOBALS.window   = NULL;
    GLOBALS.renderer = NULL;
    PREVIOUS_TICKS_NS = 0;
    DELTA_TIME_SECONDS = 0.0f;
    FPS = 0;

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

    GLOBALS.ready = true;
    PREVIOUS_TICKS_NS = SDL_GetTicksNS();
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
    PREVIOUS_TICKS_NS = 0;
    DELTA_TIME_SECONDS = 0.0f;
    FPS = 0;

    SDL_Quit();
}

bool is_window_ready(void)
{
    return GLOBALS.ready;
}

float get_delta_time(void)
{
    if ((SDL_WasInit(SDL_INIT_VIDEO) & SDL_INIT_VIDEO) == 0) {
        return 0.0f;
    }

    Uint64 current_ticks_ns = SDL_GetTicksNS();

    if (PREVIOUS_TICKS_NS == 0) {
        PREVIOUS_TICKS_NS = current_ticks_ns;
        DELTA_TIME_SECONDS = 0.0f;
        FPS = 0;
        return DELTA_TIME_SECONDS;
    }

    Uint64 elapsed_ticks_ns = current_ticks_ns - PREVIOUS_TICKS_NS;
    PREVIOUS_TICKS_NS = current_ticks_ns;

    DELTA_TIME_SECONDS = (float)((double)elapsed_ticks_ns / 1000000000.0);
    FPS = (DELTA_TIME_SECONDS > 0.0f) ? (int)(1.0f / DELTA_TIME_SECONDS) : 0;

    return DELTA_TIME_SECONDS;
}

int get_fps(void)
{
    return FPS;
}

SDL_Renderer *get_renderer(void)
{
    return GLOBALS.renderer;
}


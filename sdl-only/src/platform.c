#include "platform.h"
#include <SDL3/SDL.h>

// Core global state context data
typedef struct Globals {

    const char *title;                  // Window text title const pointer
    bool ready;                         // Check if window has been initialized successfully
    bool should_close;                  // Set when SDL_EVENT_QUIT is received

    SDL_Window   *window;
    SDL_Renderer *renderer;
    Uint64 previous_ticks_ns;
    float delta_time;
    int fps;

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

void init_window(int width, int height, const char *title)
{
    GLOBALS.window   = NULL;
    GLOBALS.renderer = NULL;
    GLOBALS.previous_ticks_ns = 0;
    GLOBALS.delta_time = 0.0f;
    GLOBALS.fps = 0;

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
    GLOBALS.fps = 0;

    SDL_Quit();
}

bool is_window_ready(void)
{
    return GLOBALS.ready;
}

int get_fps(void)
{
    return GLOBALS.fps;
}

SDL_Renderer *get_renderer(void)
{
    return GLOBALS.renderer;
}

bool is_key_down(int key)
{
    if (key > 0 && key < SDL_SCANCODE_COUNT)
        return GLOBALS.Keyboard.currentKeyState[key];
    return false;
}

bool is_key_pressed(int key)
{
    if (key > 0 && key < SDL_SCANCODE_COUNT)
        return GLOBALS.Keyboard.currentKeyState[key] && !GLOBALS.Keyboard.previousKeyState[key];
    return false;
}

float get_deltatime(void)
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

static SDL_MouseButtonFlags _mouse_button_mask(int button)
{
    switch (button) {
        case MOUSE_BUTTON_LEFT:   return SDL_BUTTON_LMASK;
        case MOUSE_BUTTON_RIGHT:  return SDL_BUTTON_RMASK;
        case MOUSE_BUTTON_MIDDLE: return SDL_BUTTON_MMASK;
        case MOUSE_BUTTON_SIDE:   return SDL_BUTTON_X1MASK;
        case MOUSE_BUTTON_EXTRA:  return SDL_BUTTON_X2MASK;
        default: return 0;
    }
}

void platform_begin_frame(void)
{
    // Keyboard
    SDL_memcpy(GLOBALS.Keyboard.previousKeyState, GLOBALS.Keyboard.currentKeyState, SDL_SCANCODE_COUNT * sizeof(bool));
    const bool *sdl_keys = SDL_GetKeyboardState(NULL);
    SDL_memcpy(GLOBALS.Keyboard.currentKeyState, sdl_keys, SDL_SCANCODE_COUNT * sizeof(bool));

    // Mouse
    GLOBALS.Mouse.previousButtons = GLOBALS.Mouse.currentButtons;
    GLOBALS.Mouse.currentButtons = SDL_GetMouseState(&GLOBALS.Mouse.x, &GLOBALS.Mouse.y);
    GLOBALS.Mouse.wheelY = 0.0f;
}

void platform_process_event(SDL_Event *event)
{
    if (event->type == SDL_EVENT_MOUSE_WHEEL)
        GLOBALS.Mouse.wheelY += event->wheel.y;
    else if (event->type == SDL_EVENT_QUIT)
        GLOBALS.should_close = true;
}

bool window_should_close(void)
{
    return GLOBALS.should_close;
}

bool is_mouse_button_down(int button)
{
    return (GLOBALS.Mouse.currentButtons & _mouse_button_mask(button)) != 0;
}

bool is_mouse_button_pressed(int button)
{
    SDL_MouseButtonFlags mask = _mouse_button_mask(button);
    return (GLOBALS.Mouse.currentButtons & mask) != 0 &&
           (GLOBALS.Mouse.previousButtons & mask) == 0;
}

vector2 get_mouse_position(void)
{
    return (vector2){ GLOBALS.Mouse.x, GLOBALS.Mouse.y };
}

float get_mouse_wheel_move(void)
{
    return GLOBALS.Mouse.wheelY;
}

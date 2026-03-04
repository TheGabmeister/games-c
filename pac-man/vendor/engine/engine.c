#include "engine.h"
#include <SDL3/SDL.h>

#ifndef MAX_KEYBOARD_KEYS
    #define MAX_KEYBOARD_KEYS            512        // Maximum number of keyboard keys supported
#endif
#ifndef MAX_KEY_PRESSED_QUEUE
    #define MAX_KEY_PRESSED_QUEUE         16        // Maximum number of keys in the key input queue
#endif
#ifndef MAX_CHAR_PRESSED_QUEUE
    #define MAX_CHAR_PRESSED_QUEUE        16        // Maximum number of characters in the char input queue
#endif

// Core global state context data
typedef struct Globals {
    
    const char *title;                  // Window text title const pointer
    bool ready;                         // Check if window has been initialized successfully

    SDL_Window   *window;
    SDL_Renderer *renderer;
    Uint64 previous_ticks_ns;
    float delta_time;
    int fps;

    struct {
        int exitKey;                    // Default exit key
        char currentKeyState[MAX_KEYBOARD_KEYS];        // Registers current frame key state
        char previousKeyState[MAX_KEYBOARD_KEYS];       // Registers previous frame key stat
        char keyRepeatInFrame[MAX_KEYBOARD_KEYS];       // Registers key repeats for current fram
        int keyPressedQueue[MAX_KEY_PRESSED_QUEUE];     // Input keys queue
        int keyPressedQueueCount;       // Input keys queue coun
        int charPressedQueue[MAX_CHAR_PRESSED_QUEUE];   // Input characters queue (unicode)
        int charPressedQueueCount;      // Input characters queue count
    } Keyboard;

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
    bool down = false;

    if ((key > 0) && (key < MAX_KEYBOARD_KEYS))
    {
        if (GLOBALS.Keyboard.currentKeyState[key] == 1) down = true;
    }

    return down;
}

bool is_key_pressed(int key)
{
    bool pressed = false;

    if ((key > 0) && (key < MAX_KEYBOARD_KEYS))
    {
        if ((GLOBALS.Keyboard.previousKeyState[key] == 0) && (GLOBALS.Keyboard.currentKeyState[key] == 1)) pressed = true;
    }

    return pressed;
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

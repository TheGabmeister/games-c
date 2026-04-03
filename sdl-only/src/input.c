#include "input.h"

// --- State -------------------------------------------------------------------

static struct {
    bool current[SDL_SCANCODE_COUNT];
    bool previous[SDL_SCANCODE_COUNT];
} keyboard;

static struct {
    SDL_MouseButtonFlags current;
    SDL_MouseButtonFlags previous;
    float x, y;
    float wheel_y;
} mouse;

static SDL_Scancode bindings[ACTION_COUNT][MAX_BINDINGS];

// --- Lifecycle ---------------------------------------------------------------

void input_init(void)
{
    SDL_memset(&keyboard, 0, sizeof(keyboard));
    SDL_memset(&mouse, 0, sizeof(mouse));
    SDL_memset(bindings, 0, sizeof(bindings));

    // Default bindings
    input_bind(ACTION_UP,      SDL_SCANCODE_W);
    input_bind(ACTION_UP,      SDL_SCANCODE_UP);
    input_bind(ACTION_DOWN,    SDL_SCANCODE_S);
    input_bind(ACTION_DOWN,    SDL_SCANCODE_DOWN);
    input_bind(ACTION_LEFT,    SDL_SCANCODE_A);
    input_bind(ACTION_LEFT,    SDL_SCANCODE_LEFT);
    input_bind(ACTION_RIGHT,   SDL_SCANCODE_D);
    input_bind(ACTION_RIGHT,   SDL_SCANCODE_RIGHT);
    input_bind(ACTION_CONFIRM, SDL_SCANCODE_RETURN);
    input_bind(ACTION_CONFIRM, SDL_SCANCODE_SPACE);
    input_bind(ACTION_CANCEL,  SDL_SCANCODE_ESCAPE);
}

void input_begin_frame(void)
{
    // Keyboard
    SDL_memcpy(keyboard.previous, keyboard.current, sizeof(keyboard.current));
    const bool *sdl_keys = SDL_GetKeyboardState(NULL);
    SDL_memcpy(keyboard.current, sdl_keys, SDL_SCANCODE_COUNT * sizeof(bool));

    // Mouse
    mouse.previous = mouse.current;
    mouse.current  = SDL_GetMouseState(&mouse.x, &mouse.y);
    mouse.wheel_y  = 0.0f;
}

void input_process_event(SDL_Event *event)
{
    if (event->type == SDL_EVENT_MOUSE_WHEEL)
        mouse.wheel_y += event->wheel.y;
}

// --- Binding -----------------------------------------------------------------

void input_bind(int action, SDL_Scancode key)
{
    if (action < 0 || action >= ACTION_COUNT) return;

    for (int i = 0; i < MAX_BINDINGS; i++) {
        if (bindings[action][i] == SDL_SCANCODE_UNKNOWN) {
            bindings[action][i] = key;
            return;
        }
    }
    bindings[action][MAX_BINDINGS - 1] = key;
}

void input_unbind(int action, SDL_Scancode key)
{
    if (action < 0 || action >= ACTION_COUNT) return;

    for (int i = 0; i < MAX_BINDINGS; i++) {
        if (bindings[action][i] == key) {
            bindings[action][i] = SDL_SCANCODE_UNKNOWN;
            return;
        }
    }
}

void input_clear_bindings(int action)
{
    if (action < 0 || action >= ACTION_COUNT) return;

    for (int i = 0; i < MAX_BINDINGS; i++)
        bindings[action][i] = SDL_SCANCODE_UNKNOWN;
}

// --- Action queries ----------------------------------------------------------

bool action_down(int action)
{
    if (action < 0 || action >= ACTION_COUNT) return false;

    for (int i = 0; i < MAX_BINDINGS; i++) {
        SDL_Scancode sc = bindings[action][i];
        if (sc != SDL_SCANCODE_UNKNOWN && keyboard.current[sc])
            return true;
    }
    return false;
}

bool action_pressed(int action)
{
    if (action < 0 || action >= ACTION_COUNT) return false;

    for (int i = 0; i < MAX_BINDINGS; i++) {
        SDL_Scancode sc = bindings[action][i];
        if (sc != SDL_SCANCODE_UNKNOWN && keyboard.current[sc] && !keyboard.previous[sc])
            return true;
    }
    return false;
}

// --- Raw keyboard ------------------------------------------------------------

bool is_key_down(int key)
{
    if (key > 0 && key < SDL_SCANCODE_COUNT)
        return keyboard.current[key];
    return false;
}

bool is_key_pressed(int key)
{
    if (key > 0 && key < SDL_SCANCODE_COUNT)
        return keyboard.current[key] && !keyboard.previous[key];
    return false;
}

// --- Mouse -------------------------------------------------------------------

bool is_mouse_button_down(SDL_MouseButtonFlags button)
{
    return (mouse.current & button) != 0;
}

bool is_mouse_button_pressed(SDL_MouseButtonFlags button)
{
    return (mouse.current & button) != 0 &&
           (mouse.previous & button) == 0;
}

vector2 get_mouse_position(void)
{
    return (vector2){ mouse.x, mouse.y };
}

float get_mouse_wheel_move(void)
{
    return mouse.wheel_y;
}

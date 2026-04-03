#include "input.h"
#include <SDL3/SDL.h>
#include <string.h>

static bool keys_current[SDL_SCANCODE_COUNT];
static bool keys_previous[SDL_SCANCODE_COUNT];

static float mouse_x, mouse_y;
static SDL_MouseButtonFlags mouse_current;
static SDL_MouseButtonFlags mouse_previous;

void input_update(void)
{
    memcpy(keys_previous, keys_current, sizeof(keys_current));
    mouse_previous = mouse_current;

    int numkeys;
    const bool *state = SDL_GetKeyboardState(&numkeys);
    if (numkeys > SDL_SCANCODE_COUNT) numkeys = SDL_SCANCODE_COUNT;
    memcpy(keys_current, state, (size_t)numkeys * sizeof(bool));

    mouse_current = SDL_GetMouseState(&mouse_x, &mouse_y);
}

bool input_key_down(SDL_Scancode key)
{
    return keys_current[key];
}

bool input_key_pressed(SDL_Scancode key)
{
    return keys_current[key] && !keys_previous[key];
}

bool input_key_released(SDL_Scancode key)
{
    return !keys_current[key] && keys_previous[key];
}

Vec2 input_mouse_pos(void)
{
    return vec2(mouse_x, mouse_y);
}

bool input_mouse_down(int button)
{
    return (mouse_current & SDL_BUTTON_MASK(button)) != 0;
}

bool input_mouse_pressed(int button)
{
    SDL_MouseButtonFlags mask = SDL_BUTTON_MASK(button);
    return (mouse_current & mask) && !(mouse_previous & mask);
}

bool input_mouse_released(int button)
{
    SDL_MouseButtonFlags mask = SDL_BUTTON_MASK(button);
    return !(mouse_current & mask) && (mouse_previous & mask);
}

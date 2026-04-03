#ifndef INPUT_H
#define INPUT_H

#include "platform.h"

// --- Action system -----------------------------------------------------------

#define MAX_BINDINGS 4

enum Action {
    ACTION_UP,
    ACTION_DOWN,
    ACTION_LEFT,
    ACTION_RIGHT,
    ACTION_CONFIRM,
    ACTION_CANCEL,
    ACTION_COUNT
};

void input_init(void);
void input_begin_frame(void);
void input_process_event(SDL_Event *event);

// Binding
void input_bind(int action, SDL_Scancode key);
void input_unbind(int action, SDL_Scancode key);
void input_clear_bindings(int action);

// Action queries
bool action_down(int action);
bool action_pressed(int action);

// --- Raw keyboard ------------------------------------------------------------

bool is_key_down(int key);
bool is_key_pressed(int key);

// --- Mouse -------------------------------------------------------------------

bool is_mouse_button_down(SDL_MouseButtonFlags button);
bool is_mouse_button_pressed(SDL_MouseButtonFlags button);
vector2 get_mouse_position(void);
float get_mouse_wheel_move(void);

#endif // INPUT_H

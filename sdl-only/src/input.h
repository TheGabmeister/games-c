#ifndef INPUT_H
#define INPUT_H

#include "types.h"
#include <SDL3/SDL_scancode.h>

/* Call once per frame before update, after SDL_PollEvent loop */
void input_update(void);

/* Keyboard */
bool input_key_down(SDL_Scancode key);       /* held this frame */
bool input_key_pressed(SDL_Scancode key);    /* just pressed this frame */
bool input_key_released(SDL_Scancode key);   /* just released this frame */

/* Mouse (screen-space) */
Vec2 input_mouse_pos(void);
bool input_mouse_down(int button);           /* 1=left, 2=middle, 3=right */
bool input_mouse_pressed(int button);
bool input_mouse_released(int button);

#endif

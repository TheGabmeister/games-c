#ifndef MENU_H
#define MENU_H

#include <stdbool.h>
#include <SDL3/SDL.h>

void menu_init(void);
void menu_fini(void);
void menu_handle_event(const SDL_Event *event);
void menu_render(void);

#endif

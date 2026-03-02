#ifndef GUI_SYSTEM_H
#define GUI_SYSTEM_H

#include <SDL3/SDL.h>
#include <flecs.h>

void gui_system_init(SDL_Window *window, SDL_Renderer *renderer);
void gui_system_handle_event(SDL_Event *event);
void gui_system_run(ecs_world_t *world);
void gui_system_destroy(void);

#endif /* GUI_SYSTEM_H */

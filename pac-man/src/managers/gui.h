#ifndef GUI_MANAGER_H
#define GUI_MANAGER_H

#include <flecs.h>
#include <SDL3/SDL.h>

#include "../components/widget.h"

// Forward declaration — include nuklear.h if you need the full type
struct nk_context;

void gui_manager_init(ecs_world_t *world);
void gui_manager_handle_event(SDL_Event *event);
struct nk_context *gui_manager_context(void);

#endif

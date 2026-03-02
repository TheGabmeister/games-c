#ifndef HUD_SYSTEM_H
#define HUD_SYSTEM_H

#include <flecs.h>
#include <SDL3/SDL.h>

void hud_system_init(ecs_world_t *world, SDL_Renderer *renderer);
void hud_system_run(void);
void hud_system_destroy(void);
int  hud_system_get_player_lives(void);

#endif

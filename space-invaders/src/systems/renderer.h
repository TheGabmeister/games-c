#ifndef RENDERER_SYSTEM_H
#define RENDERER_SYSTEM_H

#include "../sarsa.h"
#include "../components/transform.h"
#include "../components/sprite.h"

void renderer_system_init(ecs_world_t *world);
void renderer_system_run(ecs_world_t *world, SDL_Renderer *sdl_renderer);
void renderer_system_destroy(void);

#endif // RENDERER_SYSTEM_H

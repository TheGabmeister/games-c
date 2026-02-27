#ifndef INPUT_SYSTEM_H
#define INPUT_SYSTEM_H

#include <flecs.h>
#include <SDL3/SDL.h>
#include "tags.h"
#include "../components/velocity.h"

#define PLAYER_SPEED 200.0f

void input_system_init(ecs_world_t *world);
void input_system_run(ecs_world_t *world);
void input_system_destroy(void);

#endif

#pragma once

#include <flecs.h>

void physics_system_init(ecs_world_t *world);
void physics_system_update(ecs_world_t *world, float dt);
void physics_system_shutdown(void);

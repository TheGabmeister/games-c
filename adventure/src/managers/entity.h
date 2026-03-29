#ifndef ENTITY_MANAGER_H
#define ENTITY_MANAGER_H

#include <flecs.h>
#include "../platform.h"

void entity_manager_init(ecs_world_t *world);
void entity_manager_spawn_world(ecs_world_t *world, int game_mode);
void entity_manager_clear_gameplay(ecs_world_t *world);

#endif

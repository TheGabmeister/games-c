#pragma once

#include <flecs.h>

void collision_system_init(ecs_world_t *world);

/* Detects AABB collisions, deletes involved entities, and writes their IDs into
   killed_out (up to max_killed entries). Returns the number of entities killed. */
int collision_system_update(ecs_world_t *world, ecs_entity_t *killed_out, int max_killed);

void collision_system_shutdown(void);

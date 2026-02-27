#pragma once

#include <flecs.h>
#include <stdbool.h>

/* Spawns a bullet entity with the given tag (Projectile or EnemyProjectile).
   vy > 0 moves down, vy < 0 moves up. */
ecs_entity_t bullet_spawn(ecs_world_t *world, const char *name,
                           float x, float y, float vy, ecs_entity_t tag);

/* Checks if the bullet has left [0, screen_h]. If so, deletes it and returns false. */
bool bullet_update(ecs_world_t *world, ecs_entity_t bullet, float screen_h);

#pragma once

#include <flecs.h>
#include <stdbool.h>

extern ECS_TAG_DECLARE(Projectile);

/* Spawns a bullet entity with the given tag (Projectile or EnemyProjectile).
   vy > 0 moves down, vy < 0 moves up. */
ecs_entity_t bullet_spawn(ecs_world_t *world, const char *name,
                           float x, float y, float vy, ecs_entity_t tag);


#pragma once

#include <flecs.h>
#include <SDL3/SDL.h>
#include <stdbool.h>

#define ENEMY_SHOOT_INTERVAL 5.0f

ecs_entity_t enemy_spawn(ecs_world_t *world, float x, float y, SDL_Texture *tex);

/* Ticks the shoot timer. When expired resets it, and if can_shoot is true spawns
   and returns a new enemy bullet entity. Returns 0 otherwise. */
ecs_entity_t enemy_update(ecs_world_t *world, ecs_entity_t enemy,
                           float dt, float *shoot_timer, bool can_shoot);

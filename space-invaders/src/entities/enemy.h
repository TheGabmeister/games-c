#pragma once

#include <flecs.h>
#include <SDL3/SDL.h>
#include <stdbool.h>

#define ENEMY_SHOOT_INTERVAL 5.0f

extern ECS_TAG_DECLARE(Enemy);
extern ECS_TAG_DECLARE(EnemyProjectile);

ecs_entity_t enemy_spawn(ecs_world_t *world, float x, float y, SDL_Texture *tex);


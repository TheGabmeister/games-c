#pragma once

#include <flecs.h>
#include <SDL3/SDL.h>

ecs_entity_t player_spawn(ecs_world_t *world, float x, float y, SDL_Texture *tex);

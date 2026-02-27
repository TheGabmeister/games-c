#pragma once

#include <SDL3/SDL.h>
#include <flecs.h>

typedef struct {
    SDL_Texture *texture; /* NULL = draw as filled rect */
    SDL_Color    color;
} Sprite;

extern ECS_COMPONENT_DECLARE(Sprite);

#ifndef SPRITE_COMPONENT_H
#define SPRITE_COMPONENT_H

#include <SDL3/SDL.h>
#include <flecs.h>

typedef struct {
    SDL_Texture *texture; /* NULL = draw as filled rect */
    SDL_Color    color;
} Sprite;

ECS_COMPONENT_DECLARE(Sprite);

#endif
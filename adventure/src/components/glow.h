#ifndef GLOW_COMPONENT_H
#define GLOW_COMPONENT_H

#include <flecs.h>
#include <SDL3/SDL.h>

typedef struct Glow
{
    SDL_Color base_color;
    int       layers;
    float     radius;
} Glow;

ECS_COMPONENT_DECLARE(Glow);

#endif

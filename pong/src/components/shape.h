#ifndef SHAPE_COMPONENT_H
#define SHAPE_COMPONENT_H

#include <flecs.h>
#include <SDL3/SDL.h>
#include "../platform.h"

typedef struct Shape
{
    float width;
    float height;
    SDL_Color color;
} Shape;

ECS_COMPONENT_DECLARE(Shape);

#endif

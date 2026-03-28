#ifndef SHAPE_COMPONENT_H
#define SHAPE_COMPONENT_H

#include <flecs.h>
#include <SDL3/SDL.h>
#include "../platform.h"

typedef enum ShapeType
{
    SHAPE_RECTANGLE,
    SHAPE_CIRCLE,
    SHAPE_TRIANGLE
} ShapeType;

typedef struct Shape
{
    ShapeType type;
    SDL_Color color;
    union
    {
        struct { float width; float height; } rectangle;
        struct { float radius; } circle;
        struct { float base; float height; } triangle;
    };
} Shape;

ECS_COMPONENT_DECLARE(Shape);

#endif

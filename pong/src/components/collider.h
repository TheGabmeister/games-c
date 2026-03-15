#ifndef COLLIDER_COMPONENT_H
#define COLLIDER_COMPONENT_H

#include <flecs.h>
#include <stdint.h>

typedef enum ColliderType
{
    COLLIDER_RECT,
    COLLIDER_CIRCLE
} ColliderType;

typedef struct Collider
{
    uint32_t     layer;
    uint32_t     mask;
    ColliderType type;
    union
    {
        struct { float width; float height; } rect;
        struct { float radius; } circle;
    };
} Collider;

ECS_COMPONENT_DECLARE(Collider);

#endif

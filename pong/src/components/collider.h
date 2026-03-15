#ifndef COLLIDER_COMPONENT_H
#define COLLIDER_COMPONENT_H

#include <flecs.h>
#include <stdint.h>

typedef struct Collider
{
    uint32_t layer;
    uint32_t mask;
} Collider;

ECS_COMPONENT_DECLARE(Collider);

#endif

#ifndef COLLISION_COMPONENT_H
#define COLLISION_COMPONENT_H

#include <flecs.h>
#include "../platform.h"

typedef struct Collision
{
    ecs_entity_t other;
    vector2      normal;
} Collision;

ECS_COMPONENT_DECLARE(Collision);

#endif

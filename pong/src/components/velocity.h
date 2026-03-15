#ifndef VELOCITY_COMPONENT_H
#define VELOCITY_COMPONENT_H

#include <flecs.h>
#include "../platform.h"

typedef struct Velocity
{
    vector2 value;
} Velocity;

ECS_COMPONENT_DECLARE(Velocity);

#endif

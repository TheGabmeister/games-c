#ifndef VELOCITY_COMPONENT_H
#define VELOCITY_COMPONENT_H

#include <flecs.h>

typedef struct { float x, y; } Velocity;

ECS_COMPONENT_DECLARE(Velocity);

#endif
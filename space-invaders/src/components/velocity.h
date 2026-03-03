#ifndef VELOCITY_COMPONENT_H
#define VELOCITY_COMPONENT_H

#include <cglm/cglm.h>
#include <flecs.h>

typedef struct { float speed; vec2 direction; } Velocity;

ECS_COMPONENT_DECLARE(Velocity);

#endif
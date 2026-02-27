#pragma once

#include <flecs.h>

typedef struct { float x, y; } Velocity;

extern ECS_COMPONENT_DECLARE(Velocity);

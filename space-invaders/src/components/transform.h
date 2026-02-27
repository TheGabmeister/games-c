#ifndef TRANSFORM_COMPONENT_H
#define TRANSFORM_COMPONENT_H

#include <flecs.h>

typedef struct { float x, y; } Position;
typedef struct { float r; } Rotation;
typedef struct { float w, h; } Scale;

ECS_COMPONENT_DECLARE(Position);
ECS_COMPONENT_DECLARE(Rotation);
ECS_COMPONENT_DECLARE(Scale);

#endif
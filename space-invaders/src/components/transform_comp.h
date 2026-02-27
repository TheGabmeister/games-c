#pragma once

#include <flecs.h>

typedef struct { float x, y; } Position;
typedef struct { float w, h; } Size;

extern ECS_COMPONENT_DECLARE(Position);
extern ECS_COMPONENT_DECLARE(Size);

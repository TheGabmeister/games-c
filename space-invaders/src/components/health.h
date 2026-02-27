#pragma once

#include <flecs.h>

typedef struct { int current, max; } Health;

extern ECS_COMPONENT_DECLARE(Health);

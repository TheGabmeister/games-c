#ifndef HEALTH_COMPONENT_H
#define HEALTH_COMPONENT_H

#include <flecs.h>

typedef struct { int current, max; } Health;

ECS_COMPONENT_DECLARE(Health);

#endif
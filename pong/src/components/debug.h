#ifndef DEBUG_COMPONENT_H
#define DEBUG_COMPONENT_H

#include <flecs.h>
#include "../platform.h"

typedef struct Debug
{
    bool show_colliders;
} Debug;

ECS_COMPONENT_DECLARE(Debug);

#endif

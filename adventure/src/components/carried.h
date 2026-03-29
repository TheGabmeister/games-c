#ifndef CARRIED_COMPONENT_H
#define CARRIED_COMPONENT_H

#include <flecs.h>
#include "../platform.h"

typedef struct Carried
{
    ecs_entity_t carrier;
    vector2      local_offset;
} Carried;

ECS_COMPONENT_DECLARE(Carried);

#endif

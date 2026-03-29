#ifndef BAT_COMPONENT_H
#define BAT_COMPONENT_H

#include <flecs.h>
#include "../platform.h"

typedef struct Bat
{
    ecs_entity_t carried_entity;
    float        heading_x;
    float        heading_y;
    float        retarget_timer;
    float        swap_cooldown;
    float        speed;
} Bat;

ECS_COMPONENT_DECLARE(Bat);

#endif

#ifndef DRAGON_COMPONENT_H
#define DRAGON_COMPONENT_H

#include <flecs.h>

typedef enum DragonType {
    DRAGON_YORGLE  = 0,
    DRAGON_GRUNDLE = 1,
    DRAGON_RHINDLE = 2,
    DRAGON_TYPE_COUNT
} DragonType;

typedef enum DragonState {
    DRAGON_IDLE,
    DRAGON_CHASE,
    DRAGON_FLEE,
    DRAGON_BITE,
    DRAGON_DEAD
} DragonState;

typedef struct Dragon
{
    DragonType  type;
    DragonState state;
    int         home_room;
    float       speed;
    float       aggro_range;
    float       flee_range;
    float       bite_timer;
    float       grace_timer;
    float       wander_timer;
    float       wander_dir_x;
    float       wander_dir_y;
} Dragon;

ECS_COMPONENT_DECLARE(Dragon);

#endif

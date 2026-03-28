#ifndef BALL_COMPONENT_H
#define BALL_COMPONENT_H

#include <flecs.h>
#include "../platform.h"

typedef struct Ball
{
    float   speed;
    vector2 spawn;
} Ball;

ECS_COMPONENT_DECLARE(Ball);

#endif

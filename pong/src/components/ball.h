#ifndef BALL_COMPONENT_H
#define BALL_COMPONENT_H

#include <flecs.h>

typedef struct Ball
{
    float speed;
} Ball;

ECS_COMPONENT_DECLARE(Ball);

#endif

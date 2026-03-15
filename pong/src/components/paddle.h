#ifndef PADDLE_COMPONENT_H
#define PADDLE_COMPONENT_H

#include <flecs.h>

typedef struct Paddle
{
    int player;
} Paddle;

ECS_COMPONENT_DECLARE(Paddle);

#endif

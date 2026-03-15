#ifndef SCORE_COMPONENT_H
#define SCORE_COMPONENT_H

#include <flecs.h>

typedef struct Score
{
    int p1;
    int p2;
} Score;

ECS_COMPONENT_DECLARE(Score);

#endif

#ifndef GOAL_SCORED_COMPONENT_H
#define GOAL_SCORED_COMPONENT_H

#include <flecs.h>

typedef struct GoalScored
{
    int player;
} GoalScored;

ECS_COMPONENT_DECLARE(GoalScored);

#endif

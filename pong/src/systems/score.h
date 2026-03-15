#ifndef SCORE_SYSTEM_H
#define SCORE_SYSTEM_H

#include <flecs.h>

void goal_clear(ecs_iter_t *it);
void score_observers_init(ecs_world_t *world);

#endif

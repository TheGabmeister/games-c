#ifndef COLLISION_SYSTEM_H
#define COLLISION_SYSTEM_H

#include <flecs.h>

void collision_clear(ecs_iter_t *it);
void collision_detect(ecs_iter_t *it);
void apply_bounce(ecs_iter_t *it);

#endif

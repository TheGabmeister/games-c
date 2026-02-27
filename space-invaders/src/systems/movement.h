#ifndef MOVEMENT_SYSTEM_H
#define MOVEMENT_SYSTEM_H

#include "../sarsa.h"
#include "../components/transform.h"
#include "../components/velocity.h"

void movement_system_init(ecs_world_t *world);
void movement_system_run(ecs_world_t *world, float dt);
void movement_system_destroy(void);

#endif

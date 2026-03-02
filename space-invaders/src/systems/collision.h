#ifndef COLLISION_SYSTEM_H
#define COLLISION_SYSTEM_H

#include <flecs.h>

void collision_system_init(ecs_world_t *world);
void collision_system_run(ecs_world_t *world);
void collision_system_destroy(void);

#endif /* COLLISION_SYSTEM_H */

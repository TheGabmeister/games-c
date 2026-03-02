#ifndef COMBAT_SYSTEM_H
#define COMBAT_SYSTEM_H

#include <flecs.h>

void combat_system_init(ecs_world_t *world);
void combat_system_run(ecs_world_t *world);
void combat_system_destroy(void);

#endif /* COMBAT_SYSTEM_H */

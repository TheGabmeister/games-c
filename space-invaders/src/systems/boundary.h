#ifndef BOUNDARY_SYSTEM_H
#define BOUNDARY_SYSTEM_H

#include <flecs.h>

void boundary_system_init(ecs_world_t *world);
void boundary_system_run(ecs_world_t *world);
void boundary_system_destroy(void);

#endif /* BOUNDARY_SYSTEM_H */

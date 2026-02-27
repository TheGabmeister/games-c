#ifndef INPUT_SYSTEM_H
#define INPUT_SYSTEM_H

#include <flecs.h>

void input_system_init(ecs_world_t *world);
void input_system_run(ecs_world_t *world);
void input_system_destroy(void);

#endif

#ifndef ENEMY_MOVEMENT_SYSTEM_H
#define ENEMY_MOVEMENT_SYSTEM_H

#include <flecs.h>

void enemy_movement_system_init(ecs_world_t *world);
void enemy_movement_system_run(ecs_world_t *world);
void enemy_movement_system_destroy(void);

#endif /* ENEMY_MOVEMENT_SYSTEM_H */

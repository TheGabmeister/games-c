#ifndef PLAYER_SYSTEM_H
#define PLAYER_SYSTEM_H
#include <flecs.h>
void player_intent(ecs_iter_t *it);
void carry_sync(ecs_iter_t *it);
void move_entities(ecs_iter_t *it);
#endif

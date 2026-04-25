#ifndef MARIO_H
#define MARIO_H

#include "entity.h"

extern const EntityVtab mario_vtab;

void spawn_mario(Entity entities[MAX_ENTITIES], int *mario_idx, float x, float y);
bool mario_is_stomping(Entity *mario, Entity *enemy);
void mario_take_damage(Entity *mario, Game *game);

#endif

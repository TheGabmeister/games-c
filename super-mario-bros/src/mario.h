#ifndef MARIO_H
#define MARIO_H

#include "entity.h"

extern const EntityVtab mario_vtab;

void spawn_mario(Entity entities[MAX_ENTITIES], int *mario_idx, float x, float y);

#endif

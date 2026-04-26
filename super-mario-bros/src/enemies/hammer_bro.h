#ifndef HAMMER_BRO_ENT_H
#define HAMMER_BRO_ENT_H

#include "../entity.h"

void spawn_hammer_bro(Entity entities[MAX_ENTITIES], float x, float y, int extra);
void spawn_hammer_projectile(Entity entities[MAX_ENTITIES], float x, float y, Direction dir);

#endif

#ifndef SPINY_ENT_H
#define SPINY_ENT_H

#include "../entity.h"

void spawn_spiny(Entity entities[MAX_ENTITIES], float x, float y, int extra);
void spawn_spiny_egg(Entity entities[MAX_ENTITIES], float x, float y, float vx, float vy);

#endif

#ifndef KOOPA_H
#define KOOPA_H

#include "../entity.h"

void spawn_koopa(Entity entities[MAX_ENTITIES], float x, float y, int extra);
void spawn_shell(Entity entities[MAX_ENTITIES], float x, float y, float kick_vx);

#endif

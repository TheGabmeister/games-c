#ifndef FIREBAR_H
#define FIREBAR_H

#include "../entity.h"

void spawn_firebar(Entity entities[MAX_ENTITIES], float x, float y, int extra);
bool firebar_overlaps_entity(Entity *firebar, Entity *target);

#endif

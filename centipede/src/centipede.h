#ifndef CENTIPEDE_H
#define CENTIPEDE_H

#include "game.h"

void centipede_init(Segment segments[], int level);
void centipede_update(Segment segments[], MushroomGrid *grid, float speed, float dt);
void centipede_draw(Segment segments[]);
int  centipede_active_count(Segment segments[]);
bool centipede_hit_segment(Segment segments[], int idx, MushroomGrid *grid);

#endif

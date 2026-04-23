#ifndef MUSHROOM_H
#define MUSHROOM_H

#include "game.h"

void mushroom_init(MushroomGrid *grid);
int  mushroom_hit(MushroomGrid *grid, int col, int row);
void mushroom_poison(MushroomGrid *grid, int col, int row);
bool mushroom_is_blocked(MushroomGrid *grid, int col, int row);
int  mushroom_count_in_player_area(MushroomGrid *grid);
void mushroom_draw(MushroomGrid *grid);

#endif

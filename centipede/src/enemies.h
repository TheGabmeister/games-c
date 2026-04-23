#ifndef ENEMIES_H
#define ENEMIES_H

#include "game.h"

void spider_try_spawn(Spider *spider, float *timer, int level, float dt);
void spider_update(Spider *spider, MushroomGrid *grid, float dt);
void spider_draw(Spider *spider);

void flea_try_spawn(Flea *flea, MushroomGrid *grid);
void flea_update(Flea *flea, MushroomGrid *grid, float dt);
void flea_draw(Flea *flea);

void scorpion_try_spawn(Scorpion *scorpion, float *timer, int level, float dt);
void scorpion_update(Scorpion *scorpion, MushroomGrid *grid, float dt);
void scorpion_draw(Scorpion *scorpion);

#endif

#ifndef PLAYER_H
#define PLAYER_H

#include "game.h"

void player_init(Player *p);
void player_update(Player *p, Dart *dart, MushroomGrid *grid, float dt);
void dart_update(Dart *dart, float dt);
void player_draw(Player *p);
void dart_draw(Dart *dart);

#endif

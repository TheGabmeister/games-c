#ifndef ITEMS_H
#define ITEMS_H

#include "entity.h"

void spawn_coin_entity(Entity entities[MAX_ENTITIES], float x, float y);
void spawn_mushroom(Entity entities[MAX_ENTITIES], float x, float y);
void spawn_fire_flower(Entity entities[MAX_ENTITIES], float x, float y);
void spawn_starman(Entity entities[MAX_ENTITIES], float x, float y);
void spawn_oneup(Entity entities[MAX_ENTITIES], float x, float y);
void spawn_fireball(Entity entities[MAX_ENTITIES], float x, float y, Direction dir);
void spawn_brick_debris(Entity entities[MAX_ENTITIES], float x, float y);
void spawn_coin_popup(Entity entities[MAX_ENTITIES], float x, float y);
void spawn_score_popup(Entity entities[MAX_ENTITIES], float x, float y, int score);
void spawn_balance_lift(Entity entities[MAX_ENTITIES], float x, float y, int extra);

#endif

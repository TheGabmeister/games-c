#ifndef FRUIT_H
#define FRUIT_H

#include "common.h"

typedef struct {
    bool active;
    int fruit_type;
    int points;
    float timer;
    bool score_display;
    float score_display_timer;
    int score_display_value;
} Fruit;

int fruit_type_for_level(int level);
int fruit_points_for_type(int type);
void fruit_spawn(Fruit *f, int level);
void fruit_update(Fruit *f, float dt);
void fruit_draw(Fruit *f);

#endif

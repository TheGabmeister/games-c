#ifndef INVENTORY_H
#define INVENTORY_H

#include "player.h"

typedef struct PauseState {
    int cursor_x;
    int cursor_y;
} PauseState;

struct Game;

void inventory_grant(Inventory *inv, int *health, int *max_health,
                     ItemType item, int amount);

void pause_screen_update(PauseState *state, Inventory *inventory);
void pause_screen_draw(const PauseState *state, const struct Game *game);

#endif

#ifndef INVENTORY_H
#define INVENTORY_H

#include "player.h"

typedef struct PauseState {
    int cursor_x;
    int cursor_y;
} PauseState;

struct Game;

void pause_screen_update(PauseState *state, Inventory *inventory);
void pause_screen_draw(const PauseState *state, const struct Game *game);

#endif

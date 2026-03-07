#pragma once
#include "game.h"

void score_init(GameState *gs, int starting_lives);
void score_add(GameState *gs, int points);
void score_ghost_eaten(GameState *gs);    /* uses and bumps ghost_combo */
void score_reset_combo(GameState *gs);
void score_lose_life(GameState *gs);
void score_next_level(GameState *gs);     /* increment level, update speed params */

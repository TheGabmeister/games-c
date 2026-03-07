#pragma once
#include "game.h"

/* Initialize pellet state from the maze (call after maze_init) */
void pellet_init(GameState *gs);

/* Called after maze_eat_tile returns > 0; handles power pellet logic and fruit */
void pellet_on_eat(GameState *gs, TileType eaten_type);

/* Advance frightened/flash timers on all ghosts */
void pellet_update_frightened(GameState *gs, float dt);

/* Advance fruit timer; deactivate when expired */
void pellet_update_fruit(GameState *gs, float dt);

/* Determine fruit type for a given level (1-indexed) */
FruitType pellet_fruit_for_level(int level);

/* Score value for a given fruit type */
int pellet_fruit_score(FruitType type);

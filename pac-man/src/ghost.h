#pragma once
#include "game.h"

/* Initialize all 4 ghosts to their starting positions and state */
void ghost_init_all(GameState *gs);

/* Advance the scatter/chase mode schedule timer; switch modes when due */
void ghost_update_schedule(GameState *gs, float dt);

/* Update one ghost: movement, pathfinding, house exit */
void ghost_update(GameState *gs, Ghost *g, float dt);

/* Compute scatter corner for a given ghost */
TilePos ghost_scatter_target(GhostId id);

/* Compute chase target for a given ghost */
TilePos ghost_chase_target(const GameState *gs, const Ghost *g);

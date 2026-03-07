#pragma once
#include "game.h"

/* Parse the maze template into gs->tiles and count total dots */
void maze_init(GameState *gs);

/* True if (col,row) is passable by Pac-Man (not WALL, not GHOST_DOOR, not GHOST_HOUSE) */
bool maze_passable_pacman(const GameState *gs, int col, int row);

/* True if (col,row) is passable by a ghost (ghosts can pass GHOST_DOOR and GHOST_HOUSE) */
bool maze_passable_ghost(const GameState *gs, int col, int row);

/* True if this tile is a warp tunnel cell (row==TUNNEL_ROW, col in tunnel zone) */
bool maze_is_tunnel(int col, int row);

/* Get tile type safely (returns TILE_WALL for out-of-bounds) */
TileType maze_tile(const GameState *gs, int col, int row);

/* Consume dot/pellet at tile center — sets to TILE_EMPTY, returns score delta (0 if nothing) */
int maze_eat_tile(GameState *gs, int col, int row);

/* Number of dots + power pellets remaining */
int maze_dots_remaining(const GameState *gs);

/* Convert pixel position to tile (top-left of tile contains pixel) */
TilePos pixel_to_tile(float x, float y);

/* Convert tile to pixel center */
Vector2 tile_to_pixel(int col, int row);

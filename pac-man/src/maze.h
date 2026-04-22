#ifndef MAZE_H
#define MAZE_H

#define MAZE_COLS 28
#define MAZE_ROWS 36
#define TILE_SIZE 24

#define MAZE_WIDTH  (MAZE_COLS * TILE_SIZE)
#define MAZE_HEIGHT (MAZE_ROWS * TILE_SIZE)

typedef enum {
    TILE_EMPTY = 0,
    TILE_WALL,
    TILE_DOT,
    TILE_POWER_PELLET,
    TILE_GHOST_DOOR,
    TILE_TUNNEL,
} TileType;

extern const int maze_layout[MAZE_ROWS][MAZE_COLS];

#endif

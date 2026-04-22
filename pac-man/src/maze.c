#include "common.h"

// Direction vectors: UP, LEFT, DOWN, RIGHT
const int dir_dx[] = { 0, -1, 0, 1 };
const int dir_dy[] = { -1, 0, 1, 0 };

// Maze tile helpers

int maze_tile_at(int tile_x, int tile_y) {
    if (tile_x < 0 || tile_x >= MAZE_COLS || tile_y < 0 || tile_y >= MAZE_ROWS)
        return TILE_WALL;
    return maze_layout[tile_y][tile_x];
}

static int maze_tile_at_wrapped(int tile_x, int tile_y) {
    if (tile_y >= 0 && tile_y < MAZE_ROWS) {
        if (tile_x < 0) tile_x += MAZE_COLS;
        if (tile_x >= MAZE_COLS) tile_x -= MAZE_COLS;
    }
    return maze_tile_at(tile_x, tile_y);
}

bool maze_is_walkable(int tile_x, int tile_y) {
    return maze_tile_at_wrapped(tile_x, tile_y) != TILE_WALL;
}

bool maze_is_walkable_pacman(int tile_x, int tile_y) {
    int t = maze_tile_at_wrapped(tile_x, tile_y);
    return t != TILE_WALL && t != TILE_GHOST_DOOR;
}

bool maze_is_walkable_ghost(int tile_x, int tile_y, GhostMode mode) {
    int t = maze_tile_at_wrapped(tile_x, tile_y);
    if (t == TILE_WALL) return false;
    if (t == TILE_GHOST_DOOR && mode != GHOST_EXITING && mode != GHOST_EATEN)
        return false;
    return true;
}

bool maze_is_tunnel(int tile_x, int tile_y) {
    return maze_tile_at(tile_x, tile_y) == TILE_TUNNEL;
}

// Legend:
// 0 = EMPTY, 1 = WALL, 2 = DOT, 3 = POWER_PELLET,
// 4 = GHOST_DOOR, 5 = TUNNEL

const int maze_layout[MAZE_ROWS][MAZE_COLS] = {
    // Row 0: top border
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
    // Row 1: empty HUD row
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    // Row 2: empty HUD row
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    // Row 3: top wall of playfield
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
    // Row 4: dots with power pellet corners
    {1,2,2,2,2,2,2,2,2,2,2,2,2,1,1,2,2,2,2,2,2,2,2,2,2,2,2,1},
    // Row 5
    {1,2,1,1,1,1,2,1,1,1,1,1,2,1,1,2,1,1,1,1,1,2,1,1,1,1,2,1},
    // Row 6: power pellets at (1,6) and (26,6)
    {1,3,1,1,1,1,2,1,1,1,1,1,2,1,1,2,1,1,1,1,1,2,1,1,1,1,3,1},
    // Row 7
    {1,2,1,1,1,1,2,1,1,1,1,1,2,1,1,2,1,1,1,1,1,2,1,1,1,1,2,1},
    // Row 8
    {1,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,1},
    // Row 9
    {1,2,1,1,1,1,2,1,1,2,1,1,1,1,1,1,1,1,2,1,1,2,1,1,1,1,2,1},
    // Row 10
    {1,2,1,1,1,1,2,1,1,2,1,1,1,1,1,1,1,1,2,1,1,2,1,1,1,1,2,1},
    // Row 11
    {1,2,2,2,2,2,2,1,1,2,2,2,2,1,1,2,2,2,2,1,1,2,2,2,2,2,2,1},
    // Row 12
    {1,1,1,1,1,1,2,1,1,1,1,1,0,1,1,0,1,1,1,1,1,2,1,1,1,1,1,1},
    // Row 13
    {1,1,1,1,1,1,2,1,1,1,1,1,0,1,1,0,1,1,1,1,1,2,1,1,1,1,1,1},
    // Row 14
    {1,1,1,1,1,1,2,1,1,0,0,0,0,0,0,0,0,0,0,1,1,2,1,1,1,1,1,1},
    // Row 15: ghost house top with door at (13,15) and (14,15)
    {1,1,1,1,1,1,2,1,1,0,1,1,1,4,4,1,1,1,0,1,1,2,1,1,1,1,1,1},
    // Row 16: tunnel + ghost house interior
    {5,5,5,5,5,0,2,0,0,0,1,0,0,0,0,0,0,1,0,0,0,2,0,5,5,5,5,5},
    // Row 17: ghost house interior
    {1,1,1,1,1,1,2,1,1,0,1,0,0,0,0,0,0,1,0,1,1,2,1,1,1,1,1,1},
    // Row 18: ghost house bottom
    {1,1,1,1,1,1,2,1,1,0,1,1,1,1,1,1,1,1,0,1,1,2,1,1,1,1,1,1},
    // Row 19
    {1,1,1,1,1,1,2,1,1,0,0,0,0,0,0,0,0,0,0,1,1,2,1,1,1,1,1,1},
    // Row 20
    {1,1,1,1,1,1,2,1,1,0,1,1,1,1,1,1,1,1,0,1,1,2,1,1,1,1,1,1},
    // Row 21
    {1,1,1,1,1,1,2,1,1,0,1,1,1,1,1,1,1,1,0,1,1,2,1,1,1,1,1,1},
    // Row 22
    {1,2,2,2,2,2,2,2,2,2,2,2,2,1,1,2,2,2,2,2,2,2,2,2,2,2,2,1},
    // Row 23
    {1,2,1,1,1,1,2,1,1,1,1,1,2,1,1,2,1,1,1,1,1,2,1,1,1,1,2,1},
    // Row 24
    {1,2,1,1,1,1,2,1,1,1,1,1,2,1,1,2,1,1,1,1,1,2,1,1,1,1,2,1},
    // Row 25: power pellets at (1,25) and (26,25)
    {1,3,2,2,1,1,2,2,2,2,2,2,2,0,0,2,2,2,2,2,2,2,1,1,2,2,3,1},
    // Row 26: Pac-Man start at (14, 26)
    {1,1,1,2,1,1,2,1,1,2,1,1,1,0,0,1,1,1,2,1,1,2,1,1,2,1,1,1},
    // Row 27
    {1,1,1,2,1,1,2,1,1,2,1,1,1,0,0,1,1,1,2,1,1,2,1,1,2,1,1,1},
    // Row 28
    {1,2,2,2,2,2,2,1,1,2,2,2,2,2,2,2,2,2,2,1,1,2,2,2,2,2,2,1},
    // Row 29
    {1,2,1,1,1,1,1,1,1,1,1,1,2,1,1,2,1,1,1,1,1,1,1,1,1,1,2,1},
    // Row 30
    {1,2,1,1,1,1,1,1,1,1,1,1,2,1,1,2,1,1,1,1,1,1,1,1,1,1,2,1},
    // Row 31
    {1,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,1},
    // Row 32: bottom wall
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
    // Row 33: bottom HUD
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    // Row 34: bottom HUD
    {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1},
    // Row 35: bottom border
    {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
};

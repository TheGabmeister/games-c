#include "maze.h"
#include <math.h>

/* ---- Maze template ------------------------------------------------
   28 columns x 31 rows. Each string is exactly 28 chars.
   '#' = WALL
   '.' = DOT
   'o' = POWER_PELLET
   '-' = GHOST_DOOR
   'G' = GHOST_HOUSE (interior)
   ' ' = EMPTY (tunnel exits, ghost-house open space)
-------------------------------------------------------------------- */
static const char *MAZE_TEMPLATE[MAZE_ROWS] = {
/*  0 */ "############################",
/*  1 */ "#............##............#",
/*  2 */ "#.####.#####.##.#####.####.#",
/*  3 */ "#o####.#####.##.#####.####o#",
/*  4 */ "#.####.#####.##.#####.####.#",
/*  5 */ "#..........................#",
/*  6 */ "#.####.##.########.##.####.#",
/*  7 */ "#.####.##.########.##.####.#",
/*  8 */ "#......##....##....##......#",
/*  9 */ "######.#####.##.#####.######",
/* 10 */ "######.#####.##.#####.######",
/* 11 */ "######.##          ##.######",
/* 12 */ "######.## ###--### ##.######",
/* 13 */ "######.## #GGGGGG# ##.######",
/* 14 */ "       ## #GGGGGG# ##       ",
/* 15 */ "######.## #GGGGGG# ##.######",
/* 16 */ "######.## ######## ##.######",
/* 17 */ "######.##          ##.######",
/* 18 */ "######.##.########.##.######",
/* 19 */ "######.##.########.##.######",
/* 20 */ "#............##............#",
/* 21 */ "#.####.#####.##.#####.####.#",
/* 22 */ "#.####.#####.##.#####.####.#",
/* 23 */ "#o..##................##..o#",
/* 24 */ "###.##.##.########.##.##.###",
/* 25 */ "###.##.##.########.##.##.###",
/* 26 */ "#......##....##....##......#",
/* 27 */ "#.##########.##.##########.#",
/* 28 */ "#.##########.##.##########.#",
/* 29 */ "#..........................#",
/* 30 */ "############################",
};

void maze_init(GameState *gs) {
    gs->pellets.total = 0;
    gs->pellets.eaten = 0;

    for (int r = 0; r < MAZE_ROWS; r++) {
        for (int c = 0; c < MAZE_COLS; c++) {
            char ch = MAZE_TEMPLATE[r][c];
            TileType t;
            switch (ch) {
                case '#': t = TILE_WALL;          break;
                case '.': t = TILE_DOT;           gs->pellets.total++; break;
                case 'o': t = TILE_POWER_PELLET;  gs->pellets.total++; break;
                case '-': t = TILE_GHOST_DOOR;    break;
                case 'G': t = TILE_GHOST_HOUSE;   break;
                default:  t = TILE_EMPTY;          break;
            }
            gs->tiles[r][c] = (uint8_t)t;
        }
    }
}

TileType maze_tile(const GameState *gs, int col, int row) {
    if (col < 0 || col >= MAZE_COLS || row < 0 || row >= MAZE_ROWS)
        return TILE_WALL;
    return (TileType)gs->tiles[row][col];
}

bool maze_passable_pacman(const GameState *gs, int col, int row) {
    /* Wrap horizontally for tunnel */
    if (col < 0) col = MAZE_COLS - 1;
    if (col >= MAZE_COLS) col = 0;
    TileType t = maze_tile(gs, col, row);
    return t != TILE_WALL && t != TILE_GHOST_DOOR && t != TILE_GHOST_HOUSE;
}

bool maze_passable_ghost(const GameState *gs, int col, int row) {
    if (col < 0) col = MAZE_COLS - 1;
    if (col >= MAZE_COLS) col = 0;
    TileType t = maze_tile(gs, col, row);
    return t != TILE_WALL;
}

bool maze_is_tunnel(int col, int row) {
    if (row != TUNNEL_ROW) return false;
    /* Tunnel zones: cols 0-5 and 22-27 */
    return (col >= 0 && col <= 5) || (col >= 22 && col <= 27);
}

int maze_eat_tile(GameState *gs, int col, int row) {
    if (col < 0 || col >= MAZE_COLS || row < 0 || row >= MAZE_ROWS)
        return 0;
    TileType t = (TileType)gs->tiles[row][col];
    if (t == TILE_DOT) {
        gs->tiles[row][col] = (uint8_t)TILE_EMPTY;
        gs->pellets.eaten++;
        return DOT_SCORE;
    }
    if (t == TILE_POWER_PELLET) {
        gs->tiles[row][col] = (uint8_t)TILE_EMPTY;
        gs->pellets.eaten++;
        return PELLET_SCORE;
    }
    return 0;
}

int maze_dots_remaining(const GameState *gs) {
    return gs->pellets.total - gs->pellets.eaten;
}

TilePos pixel_to_tile(float x, float y) {
    /* y is in screen space including HUD offset */
    TilePos tp;
    tp.col = (int)floorf(x / TILE_SIZE);
    tp.row = (int)floorf((y - HUD_HEIGHT) / TILE_SIZE);
    return tp;
}

Vector2 tile_to_pixel(int col, int row) {
    return (Vector2){
        col * TILE_SIZE + TILE_SIZE * 0.5f,
        HUD_HEIGHT + row * TILE_SIZE + TILE_SIZE * 0.5f
    };
}

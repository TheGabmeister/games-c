#ifndef WORLD_H
#define WORLD_H

#include "game_config.h"
#include <stdbool.h>

typedef enum TileID {
    TILE_EMPTY = 0,
    TILE_BRICK,
    TILE_HOLE,
    TILE_SOLID,
    TILE_LADDER,
    TILE_ROPE,
    TILE_GOLD,
    TILE_EXIT_LADDER,
    TILE_TRAPDOOR
} TileID;

typedef struct World {
    TileID tiles[GRID_ROWS][GRID_COLS];
    float hole_timer[GRID_ROWS][GRID_COLS];
    int gold_remaining;
    int gold_total;
    bool all_gold_collected;
    int player_spawn_r;
    int player_spawn_c;
    int guard_spawn_r[MAX_GUARD_SPAWNS];
    int guard_spawn_c[MAX_GUARD_SPAWNS];
    int guard_spawn_count;
    int guard_count;
} World;

void world_clear(World *world);
bool world_load_level(World *world, int level_index, char *error, int error_size);
void world_load_builtin(World *world);

bool world_in_bounds(int r, int c);
TileID world_tile_at(const World *world, int r, int c);
bool world_tile_is_support(TileID tile, bool exit_revealed);
bool world_tile_is_passable(TileID tile, bool exit_revealed);
const char *world_tile_name(TileID tile);

#endif

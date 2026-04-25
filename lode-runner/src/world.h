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

typedef enum PursuitDir {
    PURSUE_LEFT,
    PURSUE_RIGHT,
    PURSUE_UP,
    PURSUE_DOWN,
    PURSUE_NONE
} PursuitDir;

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

typedef struct WorldTickResult {
    bool refilled_hole;
    bool refilled_tiles[GRID_ROWS][GRID_COLS];
    bool trapped_target;
} WorldTickResult;

void world_clear(World *world);
bool world_load_level(World *world, int level_index, char *error, int error_size);
void world_load_builtin(World *world);
WorldTickResult world_update_holes(World *world, float dt, int trap_r, int trap_c);
void world_rebuild_pursuit(const World *world, int player_r, int player_c, PursuitDir pursuit[GRID_ROWS][GRID_COLS]);

bool world_in_bounds(int r, int c);
TileID world_tile_at(const World *world, int r, int c);
bool world_tile_is_support(TileID tile, bool exit_revealed);
bool world_tile_is_passable(TileID tile, bool exit_revealed);
bool world_tile_is_climbable(TileID tile, bool exit_revealed);
bool world_has_support(const World *world, int r, int c);
bool world_can_enter(const World *world, int r, int c);
bool world_can_step_side(const World *world, int r, int c);
const char *world_tile_name(TileID tile);
const char *world_pursuit_dir_name(PursuitDir dir);

#endif

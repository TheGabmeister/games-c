#ifndef TILEMAP_H
#define TILEMAP_H

#include "game_config.h"
#include "items.h"
#include "dungeon.h"
#include "raylib.h"
#include <stdbool.h>
#include <stdint.h>

typedef enum TileType {
    TILE_FLOOR = 0,
    TILE_WALL,
    TILE_WATER,
    TILE_DOOR,
    TILE_PUSHBLOCK,
    TILE_STAIRS,
    TILE_BOMBABLE_WALL,
    TILE_DOOR_CLOSED,
    TILE_TYPE_COUNT
} TileType;

typedef struct TileDef {
    TileType type;
    int sprite_index;
    bool passable;
    ItemType pass_requires;
} TileDef;

extern const TileDef tile_defs[TILE_TYPE_COUNT];

typedef struct Warp {
    int tile_col;
    int tile_row;
    char dest[WARP_DEST_MAX];
    bool active;
} Warp;

typedef struct EnemySpawn {
    int type;
    int tile_col;
    int tile_row;
} EnemySpawn;

typedef struct Screen {
    uint8_t tiles[SCREEN_TILES_Y][SCREEN_TILES_X];
    Warp warps[MAX_WARPS_PER_SCREEN];
    int warp_count;
    EnemySpawn enemy_spawns[MAX_ENEMIES_PER_SCREEN];
    int enemy_spawn_count;
    DoorMeta doors[MAX_DOORS_PER_ROOM];
    int door_count;
    ItemPlacement items[MAX_ITEMS_PER_ROOM];
    int item_count;
    bool is_shutter;
    bool is_dark;
    bool is_boss_room;
} Screen;

bool screen_load(Screen *screen, const char *path);
void screen_draw(const Screen *screen);
void screen_draw_offset(const Screen *screen, int offset_x, int offset_y);
bool screen_tile_blocked(const Screen *screen, Rectangle hitbox);
const TileDef *screen_tile_at_pixel(const Screen *screen, int px, int py);
bool screen_file_exists(int sx, int sy);
const Warp *screen_warp_at(const Screen *screen, int col, int row);

#endif

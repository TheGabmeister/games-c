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
    TILE_DOCK,
    TILE_GAP,
    TILE_HEAVY_ROCK,
    TILE_BUSH,
    TILE_TYPE_COUNT
} TileType;

typedef struct TileDef {
    TileType type;
    int sprite_index;
    bool passable;
    bool requires_interaction;
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
    int variant;
    int tile_col;
    int tile_row;
} EnemySpawn;

typedef struct CaveDialogueMeta {
    int id;
    char text[CAVE_TEXT_MAX];
    bool active;
} CaveDialogueMeta;

typedef struct CaveGiftMeta {
    int id;
    ItemType reward;
    int amount;
    char text[CAVE_TEXT_MAX];
    bool active;
} CaveGiftMeta;

typedef enum UpgradeType {
    UPGRADE_SWORD = 0,
    UPGRADE_SHIELD,
    UPGRADE_ARMOR,
    UPGRADE_BOMB_CAPACITY,
    UPGRADE_ARROW_CAPACITY,
    UPGRADE_TYPE_COUNT,
} UpgradeType;

typedef struct CaveUpgradeMeta {
    int id;
    UpgradeType upgrade_type;
    int tier;
    int required_max_health;
    char text[CAVE_TEXT_MAX];
    bool active;
} CaveUpgradeMeta;

typedef struct ShopItemMeta {
    ItemType item;
    int price;
} ShopItemMeta;

typedef struct CaveShopMeta {
    int id;
    ShopItemMeta items[SHOP_ITEM_MAX];
    int item_count;
    bool active;
} CaveShopMeta;

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
    CaveDialogueMeta dialogue;
    CaveGiftMeta gift;
    CaveUpgradeMeta upgrade;
    CaveShopMeta shop;
    int npc_col;
    int npc_row;
} Screen;

bool screen_load(Screen *screen, const char *path);
void screen_draw(const Screen *screen);
void screen_draw_offset(const Screen *screen, int offset_x, int offset_y);
bool screen_tile_blocked(const Screen *screen, Rectangle hitbox);
bool screen_tile_blocked_for_items(const Screen *screen, Rectangle hitbox,
                                   Rectangle current_hitbox, uint64_t item_flags);
const TileDef *screen_tile_at_pixel(const Screen *screen, int px, int py);
bool screen_file_exists(int sx, int sy);
const Warp *screen_warp_at(const Screen *screen, int col, int row);

#endif

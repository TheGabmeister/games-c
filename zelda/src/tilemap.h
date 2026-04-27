#ifndef TILEMAP_H
#define TILEMAP_H

#include "game_config.h"
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
    TILE_TYPE_COUNT
} TileType;

typedef enum ItemID {
    ITEM_NONE = 0,
    ITEM_COUNT
} ItemID;

typedef struct TileDef {
    TileType type;
    int sprite_index;
    bool passable;
    ItemID pass_requires;
} TileDef;

extern const TileDef tile_defs[TILE_TYPE_COUNT];

typedef struct Screen {
    uint8_t tiles[SCREEN_TILES_Y][SCREEN_TILES_X];
} Screen;

bool screen_load(Screen *screen, const char *path);
void screen_draw(const Screen *screen);
bool screen_tile_blocked(const Screen *screen, Rectangle hitbox);
const TileDef *screen_tile_at_pixel(const Screen *screen, int px, int py);

#endif

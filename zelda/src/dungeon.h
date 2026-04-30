#ifndef DUNGEON_H
#define DUNGEON_H

#include "game_config.h"
#include "items.h"
#include <stdbool.h>
#include <stdint.h>

#define DUNGEON_MAX_COLS 8
#define DUNGEON_MAX_ROWS 8

typedef enum DoorType {
    DOOR_OPEN = 0,
    DOOR_LOCKED,
    DOOR_SHUTTER,
} DoorType;

typedef struct DoorMeta {
    Direction side;
    int position;
    DoorType type;
    bool active;
} DoorMeta;

#define MAX_DOORS_PER_ROOM 4

typedef struct ItemPlacement {
    ItemType type;
    int tile_col;
    int tile_row;
    bool active;
} ItemPlacement;

#define MAX_ITEMS_PER_ROOM 4

typedef struct DungeonState {
    int id;
    int room_x, room_y;
    int entrance_room_x, entrance_room_y;
    uint64_t rooms_visited;
    uint64_t rooms_cleared;
    uint64_t shutter_opened;
    uint64_t rooms_lit;
    uint64_t rooms_exist;
    uint64_t items_collected[4];
    uint64_t doors_unlocked[4];
    bool has_map;
    bool has_compass;
    bool boss_defeated;
    bool fragment_collected;
    int boss_room_x, boss_room_y;
} DungeonState;

static inline uint64_t dungeon_room_bit(int rx, int ry) {
    return 1ULL << (ry * DUNGEON_MAX_COLS + rx);
}

static inline int dungeon_door_bit_index(int rx, int ry, Direction dir) {
    return (ry * DUNGEON_MAX_COLS + rx) * 4 + dir;
}

static inline bool dungeon_door_is_unlocked(const DungeonState *ds, int rx, int ry, Direction dir) {
    int bit = dungeon_door_bit_index(rx, ry, dir);
    return (ds->doors_unlocked[bit / 64] & (1ULL << (bit % 64))) != 0;
}

static inline void dungeon_door_set_unlocked(DungeonState *ds, int rx, int ry, Direction dir) {
    int bit = dungeon_door_bit_index(rx, ry, dir);
    ds->doors_unlocked[bit / 64] |= (1ULL << (bit % 64));
}

static inline int dungeon_item_bit_index(int rx, int ry, int item_index) {
    return (ry * DUNGEON_MAX_COLS + rx) * MAX_ITEMS_PER_ROOM + item_index;
}

static inline bool dungeon_item_is_collected(const DungeonState *ds, int rx, int ry, int item_index) {
    int bit = dungeon_item_bit_index(rx, ry, item_index);
    return (ds->items_collected[bit / 64] & (1ULL << (bit % 64))) != 0;
}

static inline void dungeon_item_set_collected(DungeonState *ds, int rx, int ry, int item_index) {
    int bit = dungeon_item_bit_index(rx, ry, item_index);
    ds->items_collected[bit / 64] |= (1ULL << (bit % 64));
}

bool dungeon_room_file_exists(int dungeon_id, int rx, int ry);
void dungeon_scan_rooms(DungeonState *ds);

#endif

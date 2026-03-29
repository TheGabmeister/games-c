#include "game_modes.h"
#include <stdlib.h>
#include <string.h>
#include <time.h>

/*=============================================================================
 * Mode 1 — Beginner
 *===========================================================================*/
static const EntitySpawn mode1_items[] = {
    { ITEM_SWORD,    ROOM_GOLD_ARMORY,      480.0f, 360.0f },
    { ITEM_GOLD_KEY, ROOM_VALLEY,           480.0f, 360.0f },
    { ITEM_CHALICE,  ROOM_OVERWORLD_SOUTH,  480.0f, 300.0f },
    { ITEM_BRIDGE,   ROOM_CROSSROADS,       480.0f, 360.0f },
};

static const EntitySpawn mode1_dragons[] = {
    { DRAGON_YORGLE, ROOM_OVERWORLD_SOUTH, 300.0f, 300.0f },
};

static const int mode1_active_rooms[] = {
    ROOM_GOLD_ENTRANCE,   /*  0 */
    ROOM_GOLD_THRONE,     /*  1 */
    ROOM_GOLD_COURTYARD,  /*  2 */
    ROOM_GOLD_ARMORY,     /*  3 */
    ROOM_OVERWORLD_NORTH, /*  4 */
    ROOM_OVERWORLD_SOUTH, /*  5 */
    ROOM_CROSSROADS,      /*  6 */
    ROOM_EASTERN_PATH,    /*  7 */
    ROOM_WESTERN_PATH,    /*  8 */
    ROOM_VALLEY,          /* 12 */
};

/*=============================================================================
 * Mode 2 — Standard (all 30 rooms, fixed placement)
 *===========================================================================*/
static const EntitySpawn mode2_items[] = {
    { ITEM_SWORD,      ROOM_GOLD_ARMORY,       480.0f, 360.0f },
    { ITEM_GOLD_KEY,   ROOM_CATACOMBS_CHAMBER, 480.0f, 360.0f },
    { ITEM_BLACK_KEY,  ROOM_WHITE_LIBRARY,     480.0f, 360.0f },
    { ITEM_WHITE_KEY,  ROOM_ANCIENT_RUINS,     480.0f, 360.0f },
    { ITEM_MAGNET,     ROOM_BLACK_TREASURY,    480.0f, 360.0f },
    { ITEM_BRIDGE,     ROOM_FOREST_EDGE,       480.0f, 360.0f },
    { ITEM_CHALICE,    ROOM_BLACK_DUNGEON,     480.0f, 360.0f },
    { ITEM_SECRET_DOT, ROOM_SECRET,            480.0f, 360.0f },
};

static const EntitySpawn mode2_dragons[] = {
    { DRAGON_YORGLE,  ROOM_OVERWORLD_SOUTH, 300.0f, 300.0f },
    { DRAGON_GRUNDLE, ROOM_BLACK_HALL,      480.0f, 360.0f },
    { DRAGON_RHINDLE, ROOM_CATACOMBS_ENTRY, 480.0f, 360.0f },
};

static const int mode2_active_rooms[] = {
    ROOM_GOLD_ENTRANCE,     /*  0 */
    ROOM_GOLD_THRONE,       /*  1 */
    ROOM_GOLD_COURTYARD,    /*  2 */
    ROOM_GOLD_ARMORY,       /*  3 */
    ROOM_OVERWORLD_NORTH,   /*  4 */
    ROOM_OVERWORLD_SOUTH,   /*  5 */
    ROOM_CROSSROADS,        /*  6 */
    ROOM_EASTERN_PATH,      /*  7 */
    ROOM_WESTERN_PATH,      /*  8 */
    ROOM_RIVER_CROSSING,    /*  9 */
    ROOM_FOREST_EDGE,       /* 10 */
    ROOM_MOUNTAIN_PASS,     /* 11 */
    ROOM_VALLEY,            /* 12 */
    ROOM_ANCIENT_RUINS,     /* 13 */
    ROOM_SWAMP_EDGE,        /* 14 */
    ROOM_BLACK_GATE,        /* 15 */
    ROOM_BLACK_HALL,        /* 16 */
    ROOM_BLACK_DUNGEON,     /* 17 */
    ROOM_BLACK_TREASURY,    /* 18 */
    ROOM_WHITE_GATE,        /* 19 */
    ROOM_WHITE_HALL,        /* 20 */
    ROOM_WHITE_TOWER,       /* 21 */
    ROOM_WHITE_LIBRARY,     /* 22 */
    ROOM_CATACOMBS_ENTRY,   /* 23 */
    ROOM_CATACOMBS_1,       /* 24 */
    ROOM_CATACOMBS_2,       /* 25 */
    ROOM_CATACOMBS_3,       /* 26 */
    ROOM_CATACOMBS_DEAD,    /* 27 */
    ROOM_CATACOMBS_CHAMBER, /* 28 */
    ROOM_SECRET,            /* 29 */
};

/*=============================================================================
 * Static mode definitions
 *===========================================================================*/
static const GameModeData s_modes[3] = {
    /* Mode 1 — Beginner */
    {
        .mode              = 1,
        .name              = "Beginner",
        .items             = mode1_items,
        .item_count        = sizeof(mode1_items) / sizeof(mode1_items[0]),
        .dragons           = mode1_dragons,
        .dragon_count      = sizeof(mode1_dragons) / sizeof(mode1_dragons[0]),
        .bat_room          = -1,
        .bat_x             = 0.0f,
        .bat_y             = 0.0f,
        .player_room       = ROOM_GOLD_ENTRANCE,
        .player_x          = 480.0f,
        .player_y          = 400.0f,
        .active_rooms      = mode1_active_rooms,
        .active_room_count = sizeof(mode1_active_rooms) / sizeof(mode1_active_rooms[0]),
        .randomize         = false,
    },
    /* Mode 2 — Standard */
    {
        .mode              = 2,
        .name              = "Standard",
        .items             = mode2_items,
        .item_count        = sizeof(mode2_items) / sizeof(mode2_items[0]),
        .dragons           = mode2_dragons,
        .dragon_count      = sizeof(mode2_dragons) / sizeof(mode2_dragons[0]),
        .bat_room          = ROOM_EASTERN_PATH,
        .bat_x             = 480.0f,
        .bat_y             = 300.0f,
        .player_room       = ROOM_GOLD_ENTRANCE,
        .player_x          = 480.0f,
        .player_y          = 400.0f,
        .active_rooms      = mode2_active_rooms,
        .active_room_count = sizeof(mode2_active_rooms) / sizeof(mode2_active_rooms[0]),
        .randomize         = false,
    },
    /* Mode 3 — Randomized (same entities/rooms as mode 2, spawns shuffled) */
    {
        .mode              = 3,
        .name              = "Randomized",
        .items             = mode2_items,
        .item_count        = sizeof(mode2_items) / sizeof(mode2_items[0]),
        .dragons           = mode2_dragons,
        .dragon_count      = sizeof(mode2_dragons) / sizeof(mode2_dragons[0]),
        .bat_room          = ROOM_EASTERN_PATH,
        .bat_x             = 480.0f,
        .bat_y             = 300.0f,
        .player_room       = ROOM_GOLD_ENTRANCE,
        .player_x          = 480.0f,
        .player_y          = 400.0f,
        .active_rooms      = mode2_active_rooms,
        .active_room_count = sizeof(mode2_active_rooms) / sizeof(mode2_active_rooms[0]),
        .randomize         = true,
    },
};

/*=============================================================================
 * Public API
 *===========================================================================*/
const GameModeData *game_mode_get(int mode)
{
    if (mode < 1 || mode > 3) return &s_modes[1]; /* default to standard */
    return &s_modes[mode - 1];
}

/*-----------------------------------------------------------------------------
 * Rooms that are valid randomization targets for items/dragons.
 * Excludes ROOM_GOLD_THRONE (chalice must not spawn there — it's the win
 * destination) and ROOM_SECRET (secret dot is never randomized).
 *---------------------------------------------------------------------------*/
static const int s_valid_spawn_rooms[] = {
    ROOM_GOLD_ENTRANCE,
    ROOM_GOLD_COURTYARD,
    ROOM_GOLD_ARMORY,
    ROOM_OVERWORLD_NORTH,
    ROOM_OVERWORLD_SOUTH,
    ROOM_CROSSROADS,
    ROOM_EASTERN_PATH,
    ROOM_WESTERN_PATH,
    ROOM_RIVER_CROSSING,
    ROOM_FOREST_EDGE,
    ROOM_MOUNTAIN_PASS,
    ROOM_VALLEY,
    ROOM_ANCIENT_RUINS,
    ROOM_SWAMP_EDGE,
    ROOM_BLACK_GATE,
    ROOM_BLACK_HALL,
    ROOM_BLACK_DUNGEON,
    ROOM_BLACK_TREASURY,
    ROOM_WHITE_GATE,
    ROOM_WHITE_HALL,
    ROOM_WHITE_TOWER,
    ROOM_WHITE_LIBRARY,
    ROOM_CATACOMBS_ENTRY,
    ROOM_CATACOMBS_1,
    ROOM_CATACOMBS_2,
    ROOM_CATACOMBS_3,
    ROOM_CATACOMBS_DEAD,
    ROOM_CATACOMBS_CHAMBER,
};
static const int s_valid_spawn_room_count =
    sizeof(s_valid_spawn_rooms) / sizeof(s_valid_spawn_rooms[0]);

/*-----------------------------------------------------------------------------
 * Check whether a room is behind a gate that requires the given key type.
 * Returns true if placing key_type in room_id would lock it behind its own gate.
 *---------------------------------------------------------------------------*/
static bool key_behind_own_gate(int key_type, int room_id)
{
    switch (key_type) {
    case ITEM_GOLD_KEY:
        /* Gold castle rooms (except entrance, which is always accessible) */
        return room_id == ROOM_GOLD_THRONE ||
               room_id == ROOM_GOLD_COURTYARD ||
               room_id == ROOM_GOLD_ARMORY;
    case ITEM_BLACK_KEY:
        return room_id == ROOM_BLACK_GATE ||
               room_id == ROOM_BLACK_HALL ||
               room_id == ROOM_BLACK_DUNGEON ||
               room_id == ROOM_BLACK_TREASURY;
    case ITEM_WHITE_KEY:
        return room_id == ROOM_WHITE_GATE ||
               room_id == ROOM_WHITE_HALL ||
               room_id == ROOM_WHITE_TOWER ||
               room_id == ROOM_WHITE_LIBRARY;
    default:
        return false;
    }
}

/*-----------------------------------------------------------------------------
 * Check whether placing an item of the given type in room_id violates any
 * placement constraint.
 *---------------------------------------------------------------------------*/
static bool placement_valid(int item_type, int room_id)
{
    /* Chalice must not spawn in throne room (that's the win destination) */
    if (item_type == ITEM_CHALICE && room_id == ROOM_GOLD_THRONE)
        return false;

    /* Keys must not spawn behind their own locked gate */
    if (key_behind_own_gate(item_type, room_id))
        return false;

    return true;
}

/*-----------------------------------------------------------------------------
 * Fisher-Yates shuffle of an int array.
 *---------------------------------------------------------------------------*/
static void shuffle_ints(int *arr, int n)
{
    for (int i = n - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        int tmp = arr[i];
        arr[i] = arr[j];
        arr[j] = tmp;
    }
}

void game_mode_get_spawns(int mode, EntitySpawn *out_items, int *out_item_count,
                          EntitySpawn *out_dragons, int *out_dragon_count)
{
    const GameModeData *data = game_mode_get(mode);

    /* Copy default spawns */
    memcpy(out_items, data->items, sizeof(EntitySpawn) * data->item_count);
    *out_item_count = data->item_count;

    memcpy(out_dragons, data->dragons, sizeof(EntitySpawn) * data->dragon_count);
    *out_dragon_count = data->dragon_count;

    if (!data->randomize)
        return;

    /* Seed RNG (caller may re-seed before calling for reproducibility) */
    static bool seeded = false;
    if (!seeded) {
        srand((unsigned)time(NULL));
        seeded = true;
    }

    /*
     * Build a shuffled pool of valid spawn rooms.  We draw rooms from this
     * pool and assign them to items/dragons, skipping any that would violate
     * placement constraints.
     */
    int pool[TOTAL_ROOMS];
    int pool_size = s_valid_spawn_room_count;
    memcpy(pool, s_valid_spawn_rooms, sizeof(int) * pool_size);
    shuffle_ints(pool, pool_size);

    int pool_idx = 0;

    /* Randomize items (secret dot is never randomized) */
    for (int i = 0; i < *out_item_count; i++) {
        if (out_items[i].type == ITEM_SECRET_DOT)
            continue;

        /* Find the next valid room from the pool */
        bool found = false;
        int start = pool_idx;
        for (int attempts = 0; attempts < pool_size; attempts++) {
            int candidate = pool[(start + attempts) % pool_size];
            if (placement_valid(out_items[i].type, candidate)) {
                out_items[i].room_id = candidate;
                /* Advance past the used index; simple wrap-around */
                pool_idx = (start + attempts + 1) % pool_size;
                found = true;
                break;
            }
        }
        /* If somehow nothing valid was found, keep the default (shouldn't happen) */
        if (!found)
            out_items[i].room_id = data->items[i].room_id;
    }

    /* Randomize dragon spawns with a fresh shuffle */
    shuffle_ints(pool, pool_size);
    pool_idx = 0;
    for (int i = 0; i < *out_dragon_count; i++) {
        if (pool_idx < pool_size) {
            out_dragons[i].room_id = pool[pool_idx++];
        }
    }
}

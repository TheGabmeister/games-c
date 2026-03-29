#ifndef GAME_MODES_H
#define GAME_MODES_H

#include "rooms.h"
#include "../components/item.h"
#include "../components/dragon.h"

typedef struct EntitySpawn {
    int type;         /* ItemType or DragonType, or -1 for player/bat */
    int room_id;
    float x, y;
} EntitySpawn;

typedef struct GameModeData {
    int mode;
    const char *name;

    /* Item spawns */
    const EntitySpawn *items;
    int item_count;

    /* Dragon spawns */
    const EntitySpawn *dragons;
    int dragon_count;

    /* Bat spawn (-1 means no bat) */
    int bat_room;
    float bat_x, bat_y;

    /* Player start */
    int player_room;
    float player_x, player_y;

    /* Which rooms are active in this mode (bitfield or array) */
    const int *active_rooms;
    int active_room_count;

    /* Randomize spawns? */
    bool randomize;
} GameModeData;

const GameModeData *game_mode_get(int mode);
void game_mode_get_spawns(int mode, EntitySpawn *out_items, int *out_item_count,
                          EntitySpawn *out_dragons, int *out_dragon_count);

#endif

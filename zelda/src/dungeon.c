#include "dungeon.h"
#include "tilemap.h"
#include "raylib.h"
#include <stdio.h>

bool dungeon_room_file_exists(int dungeon_id, int rx, int ry) {
    char path[64];
    snprintf(path, sizeof(path), "assets/dungeons/%d/%02d_%02d.txt", dungeon_id, rx, ry);
    return FileExists(path);
}

void dungeon_scan_rooms(DungeonState *ds) {
    ds->rooms_exist = 0;
    ds->boss_room_x = -1;
    ds->boss_room_y = -1;
    for (int ry = 0; ry < DUNGEON_MAX_ROWS; ry++) {
        for (int rx = 0; rx < DUNGEON_MAX_COLS; rx++) {
            char path[64];
            snprintf(path, sizeof(path), "assets/dungeons/%d/%02d_%02d.txt", ds->id, rx, ry);
            if (!FileExists(path)) continue;
            ds->rooms_exist |= dungeon_room_bit(rx, ry);

            Screen tmp;
            if (screen_load(&tmp, path) && tmp.is_boss_room) {
                ds->boss_room_x = rx;
                ds->boss_room_y = ry;
            }
        }
    }
}

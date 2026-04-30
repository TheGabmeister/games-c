#ifndef MUSIC_H
#define MUSIC_H

#include <stdbool.h>

void music_init(void);
void music_update(void);
void music_cleanup(void);

void music_enter_dungeon(int dungeon_id);
void music_exit_dungeon(void);
void music_set_boss(bool in_boss_room);
void music_set_biome(int biome_id);

#endif

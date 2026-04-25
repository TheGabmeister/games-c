#ifndef GAME_H
#define GAME_H

#include "game_config.h"
#include "world.h"
#include "raylib.h"
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

typedef enum SoundID {
    SOUND_COIN = 0,
    SOUND_COUNT
} SoundID;

typedef struct Game {
    World world;
    int level_index;
    bool level_loaded_from_file;
    char level_status[160];

    Sound sounds[SOUND_COUNT];
    bool sounds_loaded;
} Game;

void game_init(Game *game);
void game_update(Game *game);
void game_draw(Game *game);

#endif

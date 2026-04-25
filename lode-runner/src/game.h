#ifndef GAME_H
#define GAME_H

#include "game_config.h"
#include "guard.h"
#include "player.h"
#include "world.h"
#include "raylib.h"
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

typedef enum SoundID {
    SOUND_COIN = 0,
    SOUND_DIG,
    SOUND_REFILL,
    SOUND_PLAYER_DIE,
    SOUND_GUARD_FALL,
    SOUND_GUARD_DIE,
    SOUND_COUNT
} SoundID;

typedef struct Game {
    World world;
    Player player;
    Guard guards[MAX_GUARDS];
    PursuitDir pursuit[GRID_ROWS][GRID_COLS];
    int level_index;
    int score;
    int lives;
    bool level_loaded_from_file;
    char level_status[160];

    Sound sounds[SOUND_COUNT];
    bool sounds_loaded;
} Game;

void game_init(Game *game);
void game_update(Game *game);
void game_draw(Game *game);

#endif

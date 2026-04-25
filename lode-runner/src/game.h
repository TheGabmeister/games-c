#ifndef GAME_H
#define GAME_H

#include "game_config.h"
#include "guard.h"
#include "particles.h"
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
    SOUND_GOLD_COMPLETE,
    SOUND_LEVEL_CLEAR,
    SOUND_COUNT
} SoundID;

typedef enum ScreenID {
    SCREEN_TITLE,
    SCREEN_PLAY,
    SCREEN_LEVEL_CLEAR,
    SCREEN_GAME_OVER
} ScreenID;

typedef struct Game {
    World world;
    Player player;
    Guard guards[MAX_GUARDS];
    PursuitDir pursuit[GRID_ROWS][GRID_COLS];
    Particles particles;
    ScreenID screen;
    int level_index;
    int score;
    int lives;
    float screen_timer;
    float shake_timer;
    float exit_reveal_timer;
    bool exit_revealed_prev;
    bool victory;
    bool paused;
    bool quit;
    bool level_loaded_from_file;
    char level_status[160];

    Texture2D bg_texture;
    Sound sounds[SOUND_COUNT];
    bool sounds_loaded;
} Game;

void game_init(Game *game);
void game_shutdown(Game *game);
void game_update(Game *game);
void game_draw(Game *game);

#endif

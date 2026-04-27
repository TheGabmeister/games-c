#ifndef GAME_H
#define GAME_H

#include "raylib.h"
#include "game_config.h"
#include "player.h"
#include "tilemap.h"
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

typedef enum GameState {
    STATE_PLAY = 0,
    STATE_COUNT
} GameState;

typedef enum SoundID {
    SOUND_COIN = 0,
    SOUND_COUNT
} SoundID;

typedef struct Game {
    GameState state;
    Player player;
    Screen current_screen;

    Sound sounds[SOUND_COUNT];
    bool sounds_loaded;
} Game;

void game_init(Game *game);
void game_update(Game *game);
void game_draw(Game *game);

#endif

#ifndef GAME_H
#define GAME_H

#include "raylib.h"
#include "game_config.h"
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

typedef enum SoundID {
    SOUND_COIN = 0,
    SOUND_COUNT
} SoundID;

typedef struct Game {
    Vector2 player_position;
    float player_speed;
    float player_radius;
    Color player_color;

    Sound sounds[SOUND_COUNT];
    bool sounds_loaded;
} Game;

void game_init(Game *game);
void game_update(Game *game);
void game_draw(Game *game);

#endif

#ifndef GAME_H
#define GAME_H

#include "raylib.h"
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

// --- Window ---
#define WINDOW_WIDTH        1280
#define WINDOW_HEIGHT       720
#define TARGET_FPS          60

// --- Player ---
#define PLAYER_START_X      (WORLD_WIDTH * 0.5f)
#define PLAYER_START_Y      330.0f
#define PLAYER_LIVES        3

// --- Particles ---
#define MAX_PARTICLES       420

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

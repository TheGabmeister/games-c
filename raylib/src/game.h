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

void game_init(Game *game);
void game_update(Game *game);
void game_draw(Game *game);

#endif

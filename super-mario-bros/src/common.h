#ifndef COMMON_H
#define COMMON_H

#include "raylib.h"
#include <stdbool.h>
#include <math.h>

// Window
#define WINDOW_WIDTH 1200
#define WINDOW_HEIGHT 900
#define TARGET_FPS 60

// Tile grid (16x16 tiles, standard SMB tile size)
#define TILE_SIZE 16

// Physics
#define GRAVITY 980.0f
#define MAX_FALL_SPEED 600.0f

// Gamepad
#define GAMEPAD_DEADZONE 0.3f

// Particles
#define MAX_PARTICLES 64
#define PARTICLE_BASE_LIFE 0.3f
#define PARTICLE_LIFE_VARIANCE 20

// Colors
#define COLOR_BG        (Color){92, 148, 252, 255}
#define COLOR_TEXT       (Color){255, 255, 255, 255}

// Direction
typedef enum {
    DIR_NONE = -1,
    DIR_LEFT = 0,
    DIR_RIGHT,
} Direction;

typedef enum {
    STATE_TITLE,
    STATE_PLAYING,
    STATE_DYING,
    STATE_GAME_OVER,
    STATE_PAUSED,
    STATE_LEVEL_COMPLETE,
} GameState;

#endif

#ifndef COMMON_H
#define COMMON_H

#include "raylib.h"
#include <stdbool.h>
#include <math.h>

// Window
#define WINDOW_WIDTH 1280
#define WINDOW_HEIGHT 960
#define TARGET_FPS 60

// Tile grid (64x64 tiles, 20x15 visible)
#define TILE_SIZE 64
#define TILES_X (WINDOW_WIDTH / TILE_SIZE)
#define TILES_Y (WINDOW_HEIGHT / TILE_SIZE)

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

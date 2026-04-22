#ifndef COMMON_H
#define COMMON_H

#include "raylib.h"
#include <stdbool.h>
#include <math.h>
#include <string.h>

// Window
#define WINDOW_WIDTH 1280
#define WINDOW_HEIGHT 960
#define TARGET_FPS 60

// Tile grid (64x64 tiles, 20x15 visible)
#define TILE_SIZE 64
#define TILES_X (WINDOW_WIDTH / TILE_SIZE)
#define TILES_Y (WINDOW_HEIGHT / TILE_SIZE)

// Entity limits
#define MAX_ENTITIES 128

// Mario physics (scaled for 64px tiles)
#define MARIO_WALK_SPEED    360.0f
#define MARIO_RUN_SPEED     600.0f
#define MARIO_ACCEL         1800.0f
#define MARIO_DECEL         1600.0f
#define MARIO_SKID_DECEL    2800.0f
#define MARIO_JUMP_VEL      -1680.0f
#define MARIO_JUMP_SUSTAIN  -300.0f
#define MARIO_STOMP_BOUNCE  -600.0f

// General physics
#define GRAVITY         3920.0f
#define MAX_FALL_SPEED  2400.0f

// Enemy physics
#define GOOMBA_SPEED        120.0f
#define KOOPA_SPEED         120.0f
#define SHELL_SPEED         600.0f
#define MUSHROOM_SPEED      180.0f
#define STARMAN_SPEED       240.0f
#define STARMAN_BOUNCE_VEL  -800.0f
#define FIREBALL_SPEED_X    500.0f
#define FIREBALL_BOUNCE_VEL -600.0f

// Mario sizes (in pixels)
#define MARIO_SMALL_W   48
#define MARIO_SMALL_H   48
#define MARIO_BIG_W     48
#define MARIO_BIG_H     96

// Entity sizes
#define ENEMY_W         48
#define ENEMY_H         48
#define KOOPA_HEIGHT    64
#define SHELL_W         48
#define SHELL_H         44
#define ITEM_W          48
#define ITEM_H          48
#define FIREBALL_SIZE   16
#define COIN_W          32
#define COIN_H          48

// Camera
#define CAMERA_THRESHOLD  (WINDOW_WIDTH * 0.4f)

// Timer
#define LEVEL_TIME          400
#define TIMER_TICK_RATE     2.5f

// Gamepad
#define GAMEPAD_DEADZONE 0.3f

// Particles
#define MAX_PARTICLES 64
#define PARTICLE_BASE_LIFE 0.3f
#define PARTICLE_LIFE_VARIANCE 20

// Invincibility after damage
#define DAMAGE_INVINCIBLE_TIME 2.0f

// Star invincibility
#define STAR_DURATION 10.0f

// Dying state duration
#define DEATH_ANIM_TIME 2.0f

// Game over display time
#define GAME_OVER_TIME 3.0f

// Level complete walk time
#define LEVEL_COMPLETE_TIME 3.0f

// Max fireballs active at once
#define MAX_FIREBALLS 2

// Score values
#define SCORE_GOOMBA_STOMP  100
#define SCORE_KOOPA_STOMP   100
#define SCORE_FIREBALL_KILL 200
#define SCORE_SHELL_KILL    100
#define SCORE_COIN          200
#define SCORE_POWERUP       1000
#define SCORE_STAR          1000
#define SCORE_FLAGPOLE_LOW  100
#define SCORE_FLAGPOLE_MID  400
#define SCORE_FLAGPOLE_HIGH 800
#define SCORE_FLAGPOLE_TOP  5000

// Block content types
#define BLOCK_COIN          1
#define BLOCK_MUSHROOM      2
#define BLOCK_FIRE_FLOWER   3
#define BLOCK_STARMAN       4
#define BLOCK_ONEUP         5
#define BLOCK_MULTI_COIN    6

// Colors
#define COLOR_BG          (Color){92, 148, 252, 255}
#define COLOR_BG_UNDER    (Color){0, 0, 0, 255}
#define COLOR_BG_CASTLE   (Color){0, 0, 0, 255}
#define COLOR_TEXT         (Color){255, 255, 255, 255}
#define COLOR_GROUND      (Color){192, 96, 0, 255}
#define COLOR_BRICK       (Color){180, 80, 0, 255}
#define COLOR_QBLOCK      (Color){255, 200, 0, 255}
#define COLOR_USED        (Color){128, 128, 128, 255}
#define COLOR_PIPE        (Color){0, 168, 0, 255}
#define COLOR_HARD        (Color){100, 100, 120, 255}
#define COLOR_MARIO       (Color){228, 0, 0, 255}
#define COLOR_MARIO_BIG   (Color){228, 0, 0, 255}
#define COLOR_GOOMBA      (Color){168, 80, 0, 255}
#define COLOR_KOOPA       (Color){0, 168, 0, 255}
#define COLOR_SHELL       (Color){0, 168, 0, 255}
#define COLOR_MUSHROOM    (Color){228, 0, 0, 255}
#define COLOR_FLOWER      (Color){255, 100, 0, 255}
#define COLOR_STARMAN     (Color){255, 255, 0, 255}
#define COLOR_ONEUP       (Color){0, 200, 0, 255}
#define COLOR_FIREBALL    (Color){255, 128, 0, 255}
#define COLOR_COIN_ENT    (Color){255, 200, 0, 255}

// Direction
typedef enum {
    DIR_NONE = -1,
    DIR_LEFT = 0,
    DIR_RIGHT,
} Direction;

// Game states
typedef enum {
    STATE_TITLE,
    STATE_PLAYING,
    STATE_DYING,
    STATE_GAME_OVER,
    STATE_PAUSED,
    STATE_LEVEL_COMPLETE,
} GameState;

// Mario power states
typedef enum {
    MARIO_SMALL,
    MARIO_BIG,
    MARIO_FIRE,
} MarioPower;

// Entity types
typedef enum {
    ENT_NONE = 0,
    ENT_MARIO,
    ENT_GOOMBA,
    ENT_KOOPA,
    ENT_SHELL,
    ENT_COIN,
    ENT_MUSHROOM,
    ENT_FIRE_FLOWER,
    ENT_STARMAN,
    ENT_ONEUP,
    ENT_FIREBALL,
    ENT_BRICK_DEBRIS,
    ENT_COIN_POPUP,
    ENT_SCORE_POPUP,
    ENT_PIRANHA,
    ENT_PARATROOPA,
    ENT_BUZZY_BEETLE,
    ENT_BULLET_BILL,
    ENT_BLOOPER,
    ENT_CHEEP_CHEEP,
    ENT_LAKITU,
    ENT_SPINY,
    ENT_HAMMER_BRO,
    ENT_BOWSER,
    ENT_FIREBAR,
    ENT_PODOBOO,
    ENT_TYPE_COUNT,
} EntityType;

// Tile types
typedef enum {
    TILE_EMPTY = 0,
    TILE_GROUND,
    TILE_BRICK,
    TILE_QUESTION,
    TILE_USED,
    TILE_HARD,
    TILE_PIPE_TL,
    TILE_PIPE_TR,
    TILE_PIPE_BL,
    TILE_PIPE_BR,
    TILE_FLAGPOLE,
    TILE_FLAGPOLE_BASE,
    TILE_CASTLE_DOOR,
    TILE_INVISIBLE,
    TILE_CORAL,
    TILE_BRIDGE,
    TILE_AXE,
    TILE_VINE_BLOCK,
    TILE_BILL_BLASTER,
    TILE_TYPE_COUNT,
} TileType;

#endif

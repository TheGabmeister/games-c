#ifndef GAME_CONFIG_H
#define GAME_CONFIG_H

// Window / layout
#define WINDOW_WIDTH        1200
#define WINDOW_HEIGHT       900
#define TARGET_FPS          60
#define HUD_HEIGHT          108
#define TILE_SIZE           36
#define GRID_COLS           30
#define GRID_ROWS           22
#define PLAY_OFFSET_X       60
#define PLAY_OFFSET_Y       108

// Player
#define PLAYER_LIVES        5
#define WALK_SPEED          6.0f
#define CLIMB_SPEED         5.0f
#define HANG_SPEED          6.0f
#define FALL_SPEED          8.0f
#define DIG_LOCK_SEC        0.25f

// Guards
#define GUARD_SPEED_MUL     0.75f
#define GUARD_HOLE_SEC      3.0f
#define GUARD_RESPAWN_SEC   1.0f
#define GUARD_PICKUP_PROB   0.5f
#define GUARD_DROP_PROB     0.08f

// Bricks / dig
#define DIG_REFILL_SEC      6.0f
#define DIG_WARN_PULSE_AT   4.5f
#define DIG_WARN_FLASH_AT   5.5f

// Scoring
#define SCORE_GOLD          100
#define SCORE_GUARD_KILL    250
#define SCORE_LEVEL_CLEAR   1500

// Capacities
#define MAX_GUARDS          8
#define MAX_LEVELS          10
#define MAX_PARTICLES       420
#define MAX_GUARD_SPAWNS    16

// Visual effects
#define SHAKE_DURATION      0.15f
#define SHAKE_MAGNITUDE     4.0f
#define EXIT_REVEAL_SEC     0.6f
#define GAMEPAD_DEADZONE    0.5f

#endif

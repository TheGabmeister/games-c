#pragma once
#include "raylib.h"
#include <stdbool.h>
#include <stdint.h>

/* ---- Dimensions ---------------------------------------------------- */
#define MAZE_COLS       28
#define MAZE_ROWS       31
#define TILE_SIZE       20
#define HUD_HEIGHT      40
#define LIVES_HEIGHT    20
#define SCREEN_W        (MAZE_COLS * TILE_SIZE)
#define SCREEN_H        (MAZE_ROWS * TILE_SIZE + HUD_HEIGHT + LIVES_HEIGHT)

/* ---- Gameplay constants -------------------------------------------- */
#define PACMAN_BASE_SPEED   0.80f   /* tiles/s level 1 */
#define GHOST_BASE_SPEED    0.75f
#define GHOST_TUNNEL_SPEED  0.40f
#define FRIGHTENED_FLASH_AT 2.0f   /* seconds before end when ghost flashes */
#define FLASH_INTERVAL      0.20f

#define TUNNEL_ROW          14
#define GHOST_DOOR_ROW      12
#define GHOST_DOOR_COL_L    13
#define GHOST_DOOR_COL_R    14

/* Ghost spawn tiles */
#define BLINKY_SPAWN_COL    13
#define BLINKY_SPAWN_ROW    11
#define PINKY_SPAWN_COL     13
#define PINKY_SPAWN_ROW     14
#define INKY_SPAWN_COL      11
#define INKY_SPAWN_ROW      14
#define CLYDE_SPAWN_COL     15
#define CLYDE_SPAWN_ROW     14

/* Ghost house re-entry target */
#define HOUSE_CENTER_COL    13
#define HOUSE_CENTER_ROW    14

/* Dot release thresholds */
#define INKY_DOT_THRESHOLD  30
#define CLYDE_DOT_THRESHOLD 60

/* Scoring */
#define DOT_SCORE           10
#define PELLET_SCORE        50
#define GHOST_BASE_SCORE    200
#define EXTRA_LIFE_SCORE    10000
#define FRUIT_TIMER         9.5f
#define FRUIT_DOT_TRIGGER1  70
#define FRUIT_DOT_TRIGGER2  170

/* Fruit spawn tile */
#define FRUIT_SPAWN_COL     13
#define FRUIT_SPAWN_ROW     17

/* ---- Enumerations -------------------------------------------------- */

typedef enum {
    TILE_EMPTY = 0,
    TILE_WALL,
    TILE_DOT,
    TILE_POWER_PELLET,
    TILE_GHOST_DOOR,
    TILE_GHOST_HOUSE,
} TileType;

typedef enum {
    DIR_NONE  = -1,
    DIR_LEFT  = 0,
    DIR_RIGHT = 1,
    DIR_UP    = 2,
    DIR_DOWN  = 3,
} Direction;

typedef enum {
    GSTATE_SCATTER    = 0,
    GSTATE_CHASE,
    GSTATE_FRIGHTENED,
    GSTATE_EATEN,
} GhostMode;

typedef enum {
    GHOST_BLINKY = 0,
    GHOST_PINKY  = 1,
    GHOST_INKY   = 2,
    GHOST_CLYDE  = 3,
    GHOST_COUNT  = 4,
} GhostId;

typedef enum {
    GAME_MENU = 0,
    GAME_READY,
    GAME_PLAYING,
    GAME_PAC_DYING,
    GAME_LEVEL_COMPLETE,
    GAME_OVER,
} GamePhase;

typedef enum {
    FRUIT_NONE = 0,
    FRUIT_CHERRY,
    FRUIT_STRAWBERRY,
    FRUIT_ORANGE,
    FRUIT_APPLE,
    FRUIT_MELON,
    FRUIT_GALAXIAN,
    FRUIT_BELL,
    FRUIT_KEY,
} FruitType;

/* ---- Simple types -------------------------------------------------- */
typedef struct { int col; int row; } TilePos;

/* ---- Player ------------------------------------------------------- */
typedef struct {
    float     x, y;
    float     speed;        /* px/s */
    Direction dir;
    Direction queued_dir;
    float     mouth_angle;  /* degrees, 0=closed 45=wide */
    bool      mouth_closing;
    float     mouth_timer;
    bool      alive;
    float     death_timer;
    float     death_angle;  /* for spinning death animation */
} Player;

/* ---- Ghost -------------------------------------------------------- */
typedef struct {
    GhostId   id;
    GhostMode mode;
    GhostMode prev_mode;
    float     x, y;         /* pixel center */
    float     speed;        /* px/s */
    Direction dir;
    Direction next_dir;
    TilePos   target;
    bool      in_house;
    bool      leaving;
    float     frightened_timer;
    float     flash_timer;
    bool      flash_visible;
    int       eaten_score;  /* score to display when eaten */
    float     eaten_display_timer; /* how long to show score */
    bool      fruit_eaten_triggered;
} Ghost;

/* ---- Mode schedule entry ------------------------------------------ */
typedef struct {
    GhostMode mode;
    float     duration; /* 0 = infinite */
} ModeEntry;

/* ---- Fruit -------------------------------------------------------- */
typedef struct {
    FruitType type;
    bool      active;
    float     timer;
    int       score_value;
    bool      trigger1_done;
    bool      trigger2_done;
} Fruit;

/* ---- Pellet state ------------------------------------------------- */
typedef struct {
    int   total;
    int   eaten;
    bool  power_active;
    float frightened_duration; /* seconds this level */
} PelletState;

/* ---- Score state -------------------------------------------------- */
typedef struct {
    int   score;
    int   high_score;
    int   lives;
    int   level;
    int   ghost_combo;   /* 0-3, index into 200/400/800/1600 */
    bool  extra_life_granted;
} ScoreState;

/* ---- Global game state -------------------------------------------- */
typedef struct {
    GamePhase   phase;
    float       phase_timer;
    float       ready_timer;

    /* Mode schedule */
    ModeEntry   schedule[8];
    int         schedule_idx;
    float       mode_timer;
    bool        schedule_done; /* true after last infinite chase phase */

    /* Sub-systems */
    Player      player;
    Ghost       ghosts[GHOST_COUNT];
    PelletState pellets;
    ScoreState  score_state;
    Fruit       fruit;

    /* Runtime tile grid (dots consumed in-place) */
    uint8_t     tiles[MAZE_ROWS][MAZE_COLS];

    /* Level parameters */
    float       pacman_speed;
    float       ghost_speed;
    float       frightened_duration;
} GameState;

#ifndef GAME_H
#define GAME_H

#include "raylib.h"
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

// --- Window ---
#define WINDOW_WIDTH    600
#define WINDOW_HEIGHT   660
#define TARGET_FPS      60

// --- Grid ---
#define GRID_COLS       30
#define GRID_ROWS       31
#define CELL_SIZE       20
#define GRID_OFFSET_X   0
#define GRID_OFFSET_Y   40

// --- Player ---
#define PLAYER_AREA_TOP_ROW  26
#define PLAYER_START_COL     15
#define PLAYER_START_ROW     29
#define PLAYER_SPEED         200.0f
#define PLAYER_LIVES         3
#define EXTRA_LIFE_INTERVAL  12000

// --- Dart ---
#define DART_SPEED           500.0f
#define DART_WIDTH           2
#define DART_HEIGHT          8

// --- Centipede ---
#define MAX_SEGMENTS         64
#define INITIAL_SEGMENTS     12
#define CENTIPEDE_BASE_SPEED 80.0f
#define HEAD_POINTS          100
#define BODY_POINTS          10

// --- Mushrooms ---
#define INITIAL_MUSHROOM_COUNT 45
#define MUSHROOM_MAX_HP      4
#define MUSHROOM_DESTROY_PTS 1
#define MUSHROOM_RESTORE_PTS 5

// --- Spider ---
#define SPIDER_SPEED         120.0f
#define SPIDER_POINTS_CLOSE  900
#define SPIDER_POINTS_MID    600
#define SPIDER_POINTS_FAR    300
#define SPIDER_CLOSE_DIST    2
#define SPIDER_MID_DIST      4

// --- Flea ---
#define FLEA_SPEED           150.0f
#define FLEA_FAST_SPEED      300.0f
#define FLEA_POINTS          200
#define FLEA_SPAWN_THRESHOLD 5

// --- Scorpion ---
#define SCORPION_SPEED       100.0f
#define SCORPION_POINTS      1000

// --- Timing ---
#define READY_DURATION       2.0f
#define DYING_DURATION       1.5f
#define GAME_OVER_DURATION   3.0f
#define RESTORE_INTERVAL     0.03f
#define LEVEL_COMPLETE_DURATION 2.0f

// --- Colors ---
#define COLOR_BG             BLACK
#define COLOR_PLAYER         (Color){0, 255, 100, 255}
#define COLOR_DART           WHITE
#define COLOR_MUSHROOM_4     (Color){0, 200, 0, 255}
#define COLOR_MUSHROOM_3     (Color){100, 200, 0, 255}
#define COLOR_MUSHROOM_2     (Color){200, 200, 0, 255}
#define COLOR_MUSHROOM_1     (Color){200, 100, 0, 255}
#define COLOR_MUSHROOM_POISON (Color){180, 0, 200, 255}
#define COLOR_HEAD           (Color){255, 50, 50, 255}
#define COLOR_BODY           (Color){0, 180, 0, 255}
#define COLOR_SPIDER         (Color){200, 50, 50, 255}
#define COLOR_FLEA           (Color){200, 200, 50, 255}
#define COLOR_SCORPION       (Color){200, 100, 50, 255}
#define COLOR_HUD            WHITE

// --- Enums ---
typedef enum {
    STATE_TITLE,
    STATE_READY,
    STATE_PLAYING,
    STATE_DYING,
    STATE_RESTORING,
    STATE_LEVEL_COMPLETE,
    STATE_GAME_OVER,
} GameState;

typedef enum {
    DIR_LEFT = 0,
    DIR_RIGHT,
    DIR_UP,
    DIR_DOWN,
} Direction;

// --- Coordinate helpers ---
static inline float col_to_px(int col) { return GRID_OFFSET_X + col * CELL_SIZE; }
static inline float row_to_px(int row) { return GRID_OFFSET_Y + row * CELL_SIZE; }
static inline int px_to_col(float x) { return (int)((x - GRID_OFFSET_X) / CELL_SIZE); }
static inline int px_to_row(float y) { return (int)((y - GRID_OFFSET_Y) / CELL_SIZE); }

// --- Mushroom ---
typedef struct {
    unsigned char cells[GRID_ROWS][GRID_COLS];
} MushroomGrid;

#define MUSH_EMPTY 0
#define MUSH_IS_PRESENT(v)  ((v) >= 1)
#define MUSH_HP(v)          ((v) <= 4 ? (v) : (v) - 4)
#define MUSH_IS_POISONED(v) ((v) >= 5 && (v) <= 8)
#define MUSH_MAKE(hp)       (hp)
#define MUSH_POISON(v)      ((v) <= 4 ? (v) + 4 : (v))

// --- Player ---
typedef struct {
    float x, y;
    int col, row;
    bool alive;
} Player;

typedef struct {
    float x, y;
    bool active;
} Dart;

// --- Centipede ---
typedef struct {
    float x, y;
    int col, row;
    Direction h_dir;
    bool is_head;
    bool active;
    bool diving;
    int next;
    int prev;
    float move_timer;
} Segment;

// --- Enemies ---
typedef struct {
    float x, y;
    float vy;
    Direction h_dir;
    bool active;
    float change_timer;
} Spider;

typedef struct {
    float x, y;
    int hp;
    bool active;
    bool fast;
    int col;
    int last_row;
} Flea;

typedef struct {
    float x, y;
    int row;
    Direction h_dir;
    bool active;
} Scorpion;

// --- Sounds ---
typedef enum {
    SND_SHOOT,
    SND_MUSHROOM_HIT,
    SND_SEGMENT_HIT,
    SND_PLAYER_DEATH,
    SND_SPIDER,
    SND_FLEA,
    SND_SCORPION,
    SND_EXTRA_LIFE,
    SND_LEVEL_COMPLETE,
    SOUND_COUNT,
} SoundID;

// --- Game ---
typedef struct {
    GameState state;
    float state_timer;
    int level;
    int score;
    int high_score;
    int lives;
    int next_extra_life;

    MushroomGrid mushrooms;

    Player player;
    Dart dart;

    Segment segments[MAX_SEGMENTS];

    Spider spider;
    Flea flea;
    Scorpion scorpion;
    float spider_spawn_timer;
    float scorpion_spawn_timer;

    int restore_row, restore_col;
    float restore_timer;

    float centipede_speed;

    Sound sounds[SOUND_COUNT];
    bool sounds_loaded;
} Game;

// --- Game functions ---
void game_init(Game *game);
void game_update(Game *game);
void game_draw(Game *game);

#endif

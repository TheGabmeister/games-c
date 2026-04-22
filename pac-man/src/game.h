#ifndef GAME_H
#define GAME_H

#include "raylib.h"
#include "maze.h"
#include <stdbool.h>

// Window
#define WINDOW_WIDTH 750
#define WINDOW_HEIGHT 1000
#define TARGET_FPS 60

// Maze offset to center it in window
#define MAZE_OFFSET_X ((WINDOW_WIDTH - MAZE_WIDTH) / 2)
#define MAZE_OFFSET_Y ((WINDOW_HEIGHT - MAZE_HEIGHT) / 2)

// Movement
#define BASE_SPEED 9.47f  // tiles/second at 100%
#define PACMAN_SPEED_NORMAL_L1 0.80f
#define PACMAN_SPEED_EATING_L1 0.71f

// Cornering tolerance in pixels
#define CORNER_TOLERANCE 10.0f

// Dot pause durations
#define DOT_PAUSE_FRAMES 1
#define PELLET_PAUSE_FRAMES 3

// Colors
#define COLOR_BG        (Color){10, 10, 26, 255}
#define COLOR_WALL      (Color){33, 33, 222, 255}
#define COLOR_WALL_GLOW (Color){33, 33, 222, 80}
#define COLOR_DOT       (Color){255, 204, 153, 255}
#define COLOR_PELLET    (Color){255, 255, 255, 255}
#define COLOR_PACMAN    (Color){255, 255, 0, 255}
#define COLOR_GHOST_DOOR (Color){255, 184, 255, 255}
#define COLOR_TEXT      (Color){255, 255, 255, 255}
#define COLOR_READY     (Color){255, 255, 0, 255}

typedef enum {
    DIR_NONE = -1,
    DIR_UP = 0,
    DIR_LEFT,
    DIR_DOWN,
    DIR_RIGHT,
} Direction;

typedef enum {
    STATE_TITLE,
    STATE_READY,
    STATE_PLAYING,
    STATE_LEVEL_COMPLETE,
    STATE_GAME_OVER,
    STATE_PAUSED,
} GameState;

typedef struct {
    float px, py;       // pixel position (center of entity)
    int tile_x, tile_y; // current tile
    Direction dir;
    Direction queued_dir;
    float speed_pct;
    int anim_frame;
    float anim_timer;
    int eat_pause_frames;
} PacMan;

typedef struct {
    GameState state;
    float state_timer;
    int level;
    int score;
    int high_score;
    int lives;
    int dots_remaining;
    int total_dots;
    bool dot_eaten[MAZE_ROWS][MAZE_COLS]; // true = consumed
    PacMan pacman;
    float pellet_flash_timer;
    bool pellet_visible;
} Game;

// game.c
void game_init(Game *game);
void game_update(Game *game);
void game_draw(Game *game);
void game_reset_level(Game *game);
void game_reset_positions(Game *game);

// maze helpers
bool maze_is_walkable(int tile_x, int tile_y);
bool maze_is_walkable_pacman(int tile_x, int tile_y);
int maze_tile_at(int tile_x, int tile_y);

// highscore
void highscore_load(Game *game);
void highscore_save(Game *game);

#endif

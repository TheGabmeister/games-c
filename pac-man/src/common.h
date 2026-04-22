#ifndef COMMON_H
#define COMMON_H

#include "raylib.h"
#include "maze.h"
#include <stdbool.h>
#include <math.h>

// Window
#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 1000
#define TARGET_FPS 60

#define MAZE_OFFSET_X ((WINDOW_WIDTH - MAZE_WIDTH) / 2)
#define MAZE_OFFSET_Y ((WINDOW_HEIGHT - MAZE_HEIGHT) / 2)

// Movement
#define BASE_SPEED 9.47f
#define CORNER_TOLERANCE 10.0f

// Dot pause durations (frames)
#define DOT_PAUSE_FRAMES 1
#define PELLET_PAUSE_FRAMES 3

// Ghost count
#define GHOST_COUNT 4
#define GHOST_BLINKY 0
#define GHOST_PINKY  1
#define GHOST_INKY   2
#define GHOST_CLYDE  3

// Ghost house positions (tile coords)
#define GHOST_HOUSE_EXIT_X 14
#define GHOST_HOUSE_EXIT_Y 14
#define GHOST_HOUSE_CENTER_X 14
#define GHOST_HOUSE_CENTER_Y 16

#define SCATTER_CHASE_PHASES 8

// Fruit
#define FRUIT_COUNT 8
#define FRUIT_TILE_X 14
#define FRUIT_TILE_Y 19
#define FRUIT_DURATION 9.5f
#define FRUIT_SCORE_DISPLAY_TIME 2.0f

// Particles
#define MAX_PARTICLES 32

// Sound
#define SOUND_COUNT 9

// Colors
#define COLOR_BG         (Color){10, 10, 26, 255}
#define COLOR_WALL       (Color){33, 33, 222, 255}
#define COLOR_WALL_GLOW  (Color){33, 33, 222, 80}
#define COLOR_WALL_WHITE (Color){255, 255, 255, 255}
#define COLOR_DOT        (Color){255, 204, 153, 255}
#define COLOR_PELLET     (Color){255, 255, 255, 255}
#define COLOR_PACMAN     (Color){255, 255, 0, 255}
#define COLOR_GHOST_DOOR (Color){255, 184, 255, 255}
#define COLOR_TEXT       (Color){255, 255, 255, 255}
#define COLOR_READY      (Color){255, 255, 0, 255}
#define COLOR_FRIGHTENED (Color){33, 33, 255, 255}
#define COLOR_FRIGHT_FLASH (Color){255, 255, 255, 255}

// Direction
typedef enum {
    DIR_NONE = -1,
    DIR_UP = 0,
    DIR_LEFT,
    DIR_DOWN,
    DIR_RIGHT,
} Direction;

typedef enum {
    GHOST_IN_HOUSE,
    GHOST_EXITING,
    GHOST_CHASE,
    GHOST_SCATTER,
    GHOST_FRIGHTENED,
    GHOST_EATEN,
} GhostMode;

typedef enum {
    GLOBAL_SCATTER,
    GLOBAL_CHASE,
} GlobalMode;

typedef enum {
    SND_DOT_A,
    SND_DOT_B,
    SND_POWER_PELLET,
    SND_GHOST_EATEN,
    SND_FRUIT,
    SND_DEATH,
    SND_EXTRA_LIFE,
    SND_READY,
    SND_MENU_SELECT,
} SoundID;

// Direction vectors
extern const int dir_dx[4];
extern const int dir_dy[4];

static inline Direction direction_opposite(Direction d) {
    if (d == DIR_NONE) return DIR_NONE;
    return (Direction)((d + 2) % 4);
}

static inline float tile_center_px(int tile) {
    return tile * TILE_SIZE + TILE_SIZE / 2.0f;
}

static inline int speed_tier(int level) {
    if (level <= 1) return 0;
    if (level <= 4) return 1;
    if (level <= 20) return 2;
    return 3;
}

// Maze helpers
int maze_tile_at(int tile_x, int tile_y);
bool maze_is_walkable(int tile_x, int tile_y);
bool maze_is_walkable_pacman(int tile_x, int tile_y);
bool maze_is_walkable_ghost(int tile_x, int tile_y, GhostMode mode);
bool maze_is_tunnel(int tile_x, int tile_y);

#endif

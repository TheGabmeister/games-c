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
#define BASE_SPEED 9.47f

// Cornering tolerance in pixels
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

// Scatter/chase phase count
#define SCATTER_CHASE_PHASES 8

// Fruit
#define FRUIT_COUNT 8
#define FRUIT_TILE_X 14
#define FRUIT_TILE_Y 20
#define FRUIT_DURATION 9.5f
#define FRUIT_SCORE_DISPLAY_TIME 2.0f

// Particles
#define MAX_PARTICLES 32

// Sound count
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
    STATE_DYING,
    STATE_LEVEL_COMPLETE,
    STATE_GAME_OVER,
    STATE_PAUSED,
} GameState;

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

typedef struct {
    float px, py;
    int tile_x, tile_y;
    Direction dir;
    Direction queued_dir;
    float speed_pct;
    int anim_frame;
    float anim_timer;
    int eat_pause_frames;
    float death_timer;
    int death_frame;
} PacMan;

typedef struct {
    float px, py;
    int tile_x, tile_y;
    Direction dir;
    GhostMode mode;
    GhostMode mode_before_fright;
    int scatter_target_x, scatter_target_y;
    int target_x, target_y;
    Color color;
    float house_bob_timer;
    int home_tile_x, home_tile_y;
    int personal_dot_counter;
    int personal_dot_limit;
    bool use_personal_counter;
} Ghost;

typedef struct {
    float x, y;
    float vx, vy;
    float life;
    float max_life;
    Color color;
    bool active;
} Particle;

typedef struct {
    bool active;
    int fruit_type;
    int points;
    float timer;
    bool score_display;
    float score_display_timer;
    int score_display_value;
} Fruit;

typedef struct {
    GameState state;
    float state_timer;
    int level;
    int score;
    int high_score;
    int lives;
    int dots_remaining;
    int total_dots;
    int dots_eaten_this_level;
    bool dot_eaten[MAZE_ROWS][MAZE_COLS];
    PacMan pacman;
    Ghost ghosts[GHOST_COUNT];

    // Pellet flash
    float pellet_flash_timer;
    bool pellet_visible;

    // Scatter/chase timing
    GlobalMode global_mode;
    int scatter_chase_phase;
    float scatter_chase_timer;

    // Frightened mode
    bool frightened_active;
    float frightened_timer;
    float frightened_duration;
    int frightened_flashes;
    int ghost_eat_combo;

    // Ghost-eaten pause
    bool ghost_eaten_pause;
    float ghost_eaten_pause_timer;
    int ghost_eaten_score_display;
    float ghost_eaten_display_x, ghost_eaten_display_y;

    // Ghost house
    int global_dot_counter;
    bool use_global_dot_counter;
    float no_dot_timer;
    float no_dot_timeout;

    // Extra life
    bool extra_life_given;

    // Level complete flash
    bool level_complete_flash_white;

    // Fruit
    Fruit fruit;
    bool fruit_spawned_70;
    bool fruit_spawned_170;

    // Particles
    Particle particles[MAX_PARTICLES];

    // Sounds
    Sound sounds[SOUND_COUNT];
    bool dot_sound_toggle;

    // Waka timer for dot sounds
    float waka_timer;
} Game;

// game.c
void game_init(Game *game);
void game_update(Game *game);
void game_draw(Game *game);
void game_load_sounds(Game *game);
void game_unload_sounds(Game *game);

// maze helpers
bool maze_is_walkable(int tile_x, int tile_y);
bool maze_is_walkable_pacman(int tile_x, int tile_y);
bool maze_is_walkable_ghost(int tile_x, int tile_y, GhostMode mode);
int maze_tile_at(int tile_x, int tile_y);
bool maze_is_tunnel(int tile_x, int tile_y);
bool maze_is_intersection(int tile_x, int tile_y);

// highscore
void highscore_load(Game *game);
void highscore_save(Game *game);

#endif

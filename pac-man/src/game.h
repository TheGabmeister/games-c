#ifndef GAME_H
#define GAME_H

#include "common.h"
#include "pacman.h"
#include "ghost.h"
#include "fruit.h"
#include "particles.h"

typedef enum {
    STATE_TITLE,
    STATE_READY,
    STATE_PLAYING,
    STATE_DYING,
    STATE_LEVEL_COMPLETE,
    STATE_GAME_OVER,
    STATE_PAUSED,
} GameState;

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

    // Cached maze wall edge/corner flags (precomputed once per level)
    unsigned char wall_flags[MAZE_ROWS][MAZE_COLS];
} Game;

void game_init(Game *game);
void game_update(Game *game);
void game_draw(Game *game);
void game_load_sounds(Game *game);
void game_unload_sounds(Game *game);
void highscore_load(Game *game);
void highscore_save(Game *game);

#endif

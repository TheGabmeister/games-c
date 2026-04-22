#ifndef GHOST_H
#define GHOST_H

#include "common.h"
#include "pacman.h"

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

void ghost_init_single(Ghost *g, int index);
void ghost_init_all(Ghost ghosts[GHOST_COUNT], int level, bool use_global_dot_counter);
void ghost_compute_target(Ghost *g, int ghost_idx, PacMan *pm, Ghost ghosts[GHOST_COUNT]);
void ghost_update_movement(Ghost *g, int ghost_idx, int level, int dots_remaining,
                           GlobalMode global_mode, PacMan *pm, Ghost ghosts[GHOST_COUNT], float dt);
void ghost_draw(Ghost *g, float frightened_timer);
void ghost_draw_all(Ghost ghosts[GHOST_COUNT], float frightened_timer);

#endif

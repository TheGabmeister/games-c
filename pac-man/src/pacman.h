#ifndef PACMAN_H
#define PACMAN_H

#include "common.h"

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

extern const float pacman_speed_eating[4];

void pacman_init(PacMan *pm);
void pacman_update(PacMan *pm, int level, bool frightened_active, float dt);
void pacman_draw(PacMan *pm, bool dying);

#endif

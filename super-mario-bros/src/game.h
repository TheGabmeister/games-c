#ifndef GAME_H
#define GAME_H

#include "common.h"
#include "particles.h"

typedef struct {
    GameState state;
    float state_timer;
    int level;
    int score;
    int lives;
    int coins;

    // Particles
    Particle particles[MAX_PARTICLES];

    // Camera
    float camera_x;
} Game;

void game_init(Game *game);
void game_update(Game *game);
void game_draw(Game *game);

#endif

#ifndef GAME_H
#define GAME_H

#include "common.h"
#include "entity.h"
#include "level.h"
#include "particles.h"

struct Game {
    GameState state;
    float state_timer;

    int world;
    int sublevel;
    int score;
    int lives;
    int coins;
    float timer;

    // Entities
    Entity entities[MAX_ENTITIES];
    int mario;

    // Level
    Level level;

    // Particles
    Particle particles[MAX_PARTICLES];

    // Camera
    float camera_x;
};

void game_init(Game *game);
void game_update(Game *game);
void game_draw(Game *game);

#endif

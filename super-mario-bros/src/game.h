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

    // Pipe warp destination (set when entering pipe)
    int warp_world, warp_sublevel;
    int warp_tx, warp_ty;

    // Castle complete bridge collapse tracking
    int bridge_collapse_tx;

    // Bill Blaster timer
    float blaster_timer;

    // Mario power to preserve across levels
    MarioPower saved_power;
};

void game_init(Game *game);
void game_update(Game *game);
void game_draw(Game *game);

#endif

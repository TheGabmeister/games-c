#ifndef PARTICLES_H
#define PARTICLES_H

#include "game_config.h"
#include "raylib.h"
#include <stdbool.h>

typedef struct Particle {
    float x, y;
    float vx, vy;
    float life;
    float max_life;
    Color color;
    bool active;
} Particle;

typedef struct Particles {
    Particle pool[MAX_PARTICLES];
} Particles;

void particles_init(Particles *p);
void particles_spawn_burst(Particles *p, float x, float y, int count, Color base, float speed, float lifetime);
void particles_update(Particles *p, float dt);
void particles_draw(const Particles *p);

#endif

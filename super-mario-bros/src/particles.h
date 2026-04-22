#ifndef PARTICLES_H
#define PARTICLES_H

#include "common.h"

typedef struct {
    float x, y;
    float vx, vy;
    float life;
    float max_life;
    Color color;
    bool active;
} Particle;

void particles_spawn(Particle particles[MAX_PARTICLES], float x, float y, Color color, int count);
void particles_update(Particle particles[MAX_PARTICLES], float dt);
void particles_draw(Particle particles[MAX_PARTICLES]);

#endif

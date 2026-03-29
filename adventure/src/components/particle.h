#ifndef PARTICLE_COMPONENT_H
#define PARTICLE_COMPONENT_H

#include <flecs.h>
#include <SDL3/SDL.h>
#include "../platform.h"

#define MAX_PARTICLES 256

typedef struct Particle
{
    float     x, y;
    float     vx, vy;
    float     lifetime;
    float     max_lifetime;
    SDL_Color color;
    int       size;
    bool      active;
} Particle;

typedef struct ParticleSystem
{
    Particle particles[MAX_PARTICLES];
    int      count;
} ParticleSystem;

ECS_COMPONENT_DECLARE(ParticleSystem);

#endif

#ifndef PARTICLE_SYSTEM_H
#define PARTICLE_SYSTEM_H
#include <flecs.h>
#include <SDL3/SDL.h>
#include "../platform.h"

void particles_init(void);
void particles_spawn_burst(float x, float y, int count, float lifetime,
                           float speed, SDL_Color color, int size);
void particles_spawn_directional(float x, float y, int count, float lifetime,
                                 float speed, float dir_x, float dir_y,
                                 SDL_Color color, int size);
void render_particles(ecs_iter_t *it);
#endif

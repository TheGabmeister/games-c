#ifndef PARTICLES_H
#define PARTICLES_H

#include "galaxian.h"

void particles_clear(Particle particles[], int max);
void particles_spawn(Particle particles[], int max, vector2 pos, int count,
                     SDL_Color color, float speed, float size);
void particles_update(Particle particles[], int max, float dt);
void particles_draw(const Particle particles[], int max, float ox, float oy);

#endif /* PARTICLES_H */

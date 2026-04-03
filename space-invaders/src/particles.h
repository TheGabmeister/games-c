#ifndef PARTICLES_H
#define PARTICLES_H

#include "game_types.h"

void SpawnParticles(Particle *pool, Vector2 pos, Color color, int count,
                    float minSpeed, float maxSpeed, float minLife, float maxLife,
                    float minSize, float maxSize);
void UpdateParticles(Particle *pool, float dt);
void DrawParticles(Particle *pool);

#endif // PARTICLES_H

#ifndef PARTICLES_H
#define PARTICLES_H

#include "game_types.h"

void ParticlesSpawn(GameContext *ctx, Vector2 pos, int count, float speed,
                    float sizeMin, float sizeMax, Color color);
void ParticlesUpdate(GameContext *ctx, float dt);
void ParticlesDraw(const GameContext *ctx);

#endif

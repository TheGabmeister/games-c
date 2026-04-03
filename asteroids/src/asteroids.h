#ifndef ASTEROIDS_H
#define ASTEROIDS_H

#include "game_types.h"

void AsteroidsSpawnWave(GameContext *ctx, int count);
void AsteroidsSplit(GameContext *ctx, int index);
int AsteroidsCountActive(const GameContext *ctx);
void AsteroidsUpdate(GameContext *ctx, float dt);
void AsteroidsDraw(const GameContext *ctx);

#endif

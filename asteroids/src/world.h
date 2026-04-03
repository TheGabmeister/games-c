#ifndef WORLD_H
#define WORLD_H

#include "game_types.h"

Vector2 WorldWrapPosition(Vector2 pos);
void WorldUpdateScreenShake(GameContext *ctx, float dt);
Camera2D WorldBuildCamera(const GameContext *ctx);

#endif

#ifndef SHIP_H
#define SHIP_H

#include "game_types.h"

void ShipInit(GameContext *ctx);
void ShipUpdate(GameContext *ctx, float dt);
void ShipDraw(const GameContext *ctx);
void ShipDrawThrustFlame(const GameContext *ctx);
Vector2 ShipGetVertex(const GameContext *ctx, int index);

#endif

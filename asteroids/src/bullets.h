#ifndef BULLETS_H
#define BULLETS_H

#include "game_types.h"

void BulletsFire(GameContext *ctx);
void BulletsUpdate(GameContext *ctx, float dt);
void BulletsDraw(const GameContext *ctx);

#endif

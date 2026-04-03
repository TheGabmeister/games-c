#ifndef GAME_FLOW_H
#define GAME_FLOW_H

#include "game_types.h"

void GameFlowBoot(GameContext *ctx);
void GameFlowUpdateTitle(GameContext *ctx, float dt);
void GameFlowUpdatePlaying(GameContext *ctx, float dt);
void GameFlowUpdateGameOver(GameContext *ctx, float dt);

#endif

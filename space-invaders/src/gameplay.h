#ifndef GAMEPLAY_H
#define GAMEPLAY_H

#include "game_types.h"

void GameInit(GameState *g);
void GameUpdate(GameState *g, float dt, int *highScore);
void GameDraw(GameState *g);
bool GameShouldEnd(GameState *g);

#endif // GAMEPLAY_H

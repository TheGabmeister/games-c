#ifndef GAMEPLAY_H
#define GAMEPLAY_H

#include "game_types.h"

typedef struct {
    float moveX;
    bool firePressed;
} GameInput;

void GameInit(GameState *g);
void GameUpdate(GameState *g, GameInput input, float dt);
bool GameShouldEnd(const GameState *g);

#endif // GAMEPLAY_H

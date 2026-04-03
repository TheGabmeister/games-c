#ifndef INPUT_H
#define INPUT_H

#include "game_types.h"

bool InputActionPressed(const GameContext *ctx, ActionType action);
bool InputActionReleased(const GameContext *ctx, ActionType action);
bool InputActionDown(const GameContext *ctx, ActionType action);
void InputSetDefault(GameContext *ctx);
void InputSetCursor(GameContext *ctx);

#endif

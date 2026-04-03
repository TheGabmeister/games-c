#include "input.h"

bool InputActionPressed(const GameContext *ctx, ActionType action)
{
    bool result = false;
    if (action < MAX_ACTION) {
        result = (IsKeyPressed(ctx->actionInputs[action].key) ||
                  IsGamepadButtonPressed(ctx->gamepadIndex, ctx->actionInputs[action].button));
    }
    return result;
}

bool InputActionReleased(const GameContext *ctx, ActionType action)
{
    bool result = false;
    if (action < MAX_ACTION) {
        result = (IsKeyReleased(ctx->actionInputs[action].key) ||
                  IsGamepadButtonReleased(ctx->gamepadIndex, ctx->actionInputs[action].button));
    }
    return result;
}

bool InputActionDown(const GameContext *ctx, ActionType action)
{
    bool result = false;
    if (action < MAX_ACTION) {
        result = (IsKeyDown(ctx->actionInputs[action].key) ||
                  IsGamepadButtonDown(ctx->gamepadIndex, ctx->actionInputs[action].button));
    }
    return result;
}

void InputSetDefault(GameContext *ctx)
{
    ctx->actionInputs[ACTION_UP].key = KEY_W;
    ctx->actionInputs[ACTION_DOWN].key = KEY_S;
    ctx->actionInputs[ACTION_LEFT].key = KEY_A;
    ctx->actionInputs[ACTION_RIGHT].key = KEY_D;
    ctx->actionInputs[ACTION_FIRE].key = KEY_SPACE;

    ctx->actionInputs[ACTION_UP].button = GAMEPAD_BUTTON_LEFT_FACE_UP;
    ctx->actionInputs[ACTION_DOWN].button = GAMEPAD_BUTTON_LEFT_FACE_DOWN;
    ctx->actionInputs[ACTION_LEFT].button = GAMEPAD_BUTTON_LEFT_FACE_LEFT;
    ctx->actionInputs[ACTION_RIGHT].button = GAMEPAD_BUTTON_LEFT_FACE_RIGHT;
    ctx->actionInputs[ACTION_FIRE].button = GAMEPAD_BUTTON_RIGHT_FACE_DOWN;
}

void InputSetCursor(GameContext *ctx)
{
    ctx->actionInputs[ACTION_UP].key = KEY_UP;
    ctx->actionInputs[ACTION_DOWN].key = KEY_DOWN;
    ctx->actionInputs[ACTION_LEFT].key = KEY_LEFT;
    ctx->actionInputs[ACTION_RIGHT].key = KEY_RIGHT;
    ctx->actionInputs[ACTION_FIRE].key = KEY_SPACE;

    ctx->actionInputs[ACTION_UP].button = GAMEPAD_BUTTON_RIGHT_FACE_UP;
    ctx->actionInputs[ACTION_DOWN].button = GAMEPAD_BUTTON_RIGHT_FACE_DOWN;
    ctx->actionInputs[ACTION_LEFT].button = GAMEPAD_BUTTON_RIGHT_FACE_LEFT;
    ctx->actionInputs[ACTION_RIGHT].button = GAMEPAD_BUTTON_RIGHT_FACE_RIGHT;
    ctx->actionInputs[ACTION_FIRE].button = GAMEPAD_BUTTON_LEFT_FACE_DOWN;
}

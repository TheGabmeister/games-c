#include "input.h"

static bool AnyKeyPressed(const ActionInput *a)
{
    for (int i = 0; i < MAX_KEYS_PER_ACTION && a->keys[i]; i++)
        if (IsKeyPressed(a->keys[i])) return true;
    return false;
}

static bool AnyKeyReleased(const ActionInput *a)
{
    for (int i = 0; i < MAX_KEYS_PER_ACTION && a->keys[i]; i++)
        if (IsKeyReleased(a->keys[i])) return true;
    return false;
}

static bool AnyKeyDown(const ActionInput *a)
{
    for (int i = 0; i < MAX_KEYS_PER_ACTION && a->keys[i]; i++)
        if (IsKeyDown(a->keys[i])) return true;
    return false;
}

bool InputActionPressed(const GameContext *ctx, ActionType action)
{
    if (action < MAX_ACTION) {
        const ActionInput *a = &ctx->actionInputs[action];
        return AnyKeyPressed(a) ||
               IsGamepadButtonPressed(ctx->gamepadIndex, a->button);
    }
    return false;
}

bool InputActionReleased(const GameContext *ctx, ActionType action)
{
    if (action < MAX_ACTION) {
        const ActionInput *a = &ctx->actionInputs[action];
        return AnyKeyReleased(a) ||
               IsGamepadButtonReleased(ctx->gamepadIndex, a->button);
    }
    return false;
}

bool InputActionDown(const GameContext *ctx, ActionType action)
{
    if (action < MAX_ACTION) {
        const ActionInput *a = &ctx->actionInputs[action];
        return AnyKeyDown(a) ||
               IsGamepadButtonDown(ctx->gamepadIndex, a->button);
    }
    return false;
}

void InputSetDefault(GameContext *ctx)
{
    ctx->actionInputs[ACTION_UP]    = (ActionInput){{ KEY_W, KEY_UP },    GAMEPAD_BUTTON_LEFT_FACE_UP };
    ctx->actionInputs[ACTION_DOWN]  = (ActionInput){{ KEY_S, KEY_DOWN },  GAMEPAD_BUTTON_LEFT_FACE_DOWN };
    ctx->actionInputs[ACTION_LEFT]  = (ActionInput){{ KEY_A, KEY_LEFT },  GAMEPAD_BUTTON_LEFT_FACE_LEFT };
    ctx->actionInputs[ACTION_RIGHT] = (ActionInput){{ KEY_D, KEY_RIGHT }, GAMEPAD_BUTTON_LEFT_FACE_RIGHT };
    ctx->actionInputs[ACTION_FIRE]  = (ActionInput){{ KEY_SPACE },        GAMEPAD_BUTTON_RIGHT_FACE_DOWN };
}

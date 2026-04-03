#include "input.h"

static bool AnyKeyPressed(const ActionInput *input)
{
    for (int i = 0; i < input->keyCount; i++) {
        if (IsKeyPressed(input->keys[i])) return true;
    }
    return false;
}

static bool AnyKeyReleased(const ActionInput *input)
{
    for (int i = 0; i < input->keyCount; i++) {
        if (IsKeyReleased(input->keys[i])) return true;
    }
    return false;
}

static bool AnyKeyDown(const ActionInput *input)
{
    for (int i = 0; i < input->keyCount; i++) {
        if (IsKeyDown(input->keys[i])) return true;
    }
    return false;
}

bool InputActionPressed(const GameContext *ctx, ActionType action)
{
    if (action <= NO_ACTION || action >= MAX_ACTION) return false;

    const ActionInput *input = &ctx->actionInputs[action];
    return AnyKeyPressed(input) ||
           IsGamepadButtonPressed(ctx->gamepadIndex, input->button);
}

bool InputActionReleased(const GameContext *ctx, ActionType action)
{
    if (action <= NO_ACTION || action >= MAX_ACTION) return false;

    const ActionInput *input = &ctx->actionInputs[action];
    return AnyKeyReleased(input) ||
           IsGamepadButtonReleased(ctx->gamepadIndex, input->button);
}

bool InputActionDown(const GameContext *ctx, ActionType action)
{
    if (action <= NO_ACTION || action >= MAX_ACTION) return false;

    const ActionInput *input = &ctx->actionInputs[action];
    return AnyKeyDown(input) ||
           IsGamepadButtonDown(ctx->gamepadIndex, input->button);
}

void InputSetDefault(GameContext *ctx)
{
    ctx->gamepadIndex = 0;
    ctx->actionInputs[ACTION_UP] = (ActionInput){
        .keys = { KEY_W, KEY_UP },
        .keyCount = 2,
        .button = GAMEPAD_BUTTON_LEFT_FACE_UP,
    };
    ctx->actionInputs[ACTION_DOWN] = (ActionInput){
        .keys = { KEY_S, KEY_DOWN },
        .keyCount = 2,
        .button = GAMEPAD_BUTTON_LEFT_FACE_DOWN,
    };
    ctx->actionInputs[ACTION_LEFT] = (ActionInput){
        .keys = { KEY_A, KEY_LEFT },
        .keyCount = 2,
        .button = GAMEPAD_BUTTON_LEFT_FACE_LEFT,
    };
    ctx->actionInputs[ACTION_RIGHT] = (ActionInput){
        .keys = { KEY_D, KEY_RIGHT },
        .keyCount = 2,
        .button = GAMEPAD_BUTTON_LEFT_FACE_RIGHT,
    };
    ctx->actionInputs[ACTION_FIRE] = (ActionInput){
        .keys = { KEY_SPACE },
        .keyCount = 1,
        .button = GAMEPAD_BUTTON_RIGHT_FACE_DOWN,
    };
}

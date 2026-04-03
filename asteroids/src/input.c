#include "input.h"

bool InputActionPressed(const GameContext *ctx, ActionType action)
{
    (void)ctx;
    switch (action) {
        case ACTION_UP:    return IsKeyPressed(KEY_W) || IsKeyPressed(KEY_UP) || IsGamepadButtonPressed(0,GAMEPAD_BUTTON_LEFT_FACE_UP);
        case ACTION_DOWN:  return IsKeyPressed(KEY_S) || IsKeyPressed(KEY_DOWN) || IsGamepadButtonPressed(0,GAMEPAD_BUTTON_LEFT_FACE_DOWN);
        case ACTION_LEFT:  return IsKeyPressed(KEY_A) || IsKeyPressed(KEY_LEFT) || IsGamepadButtonPressed(0,GAMEPAD_BUTTON_LEFT_FACE_LEFT);
        case ACTION_RIGHT: return IsKeyPressed(KEY_D) || IsKeyPressed(KEY_RIGHT) || IsGamepadButtonPressed(0,GAMEPAD_BUTTON_LEFT_FACE_RIGHT);
        case ACTION_FIRE:  return IsKeyPressed(KEY_SPACE) || IsGamepadButtonPressed(0,GAMEPAD_BUTTON_RIGHT_FACE_DOWN);
        default: return false;
    }
}

bool InputActionReleased(const GameContext *ctx, ActionType action)
{
    (void)ctx;
    switch (action) {
        case ACTION_UP:    return IsKeyReleased(KEY_W) || IsKeyReleased(KEY_UP) || IsGamepadButtonReleased(0,GAMEPAD_BUTTON_LEFT_FACE_UP);
        case ACTION_DOWN:  return IsKeyReleased(KEY_S) || IsKeyReleased(KEY_DOWN) || IsGamepadButtonReleased(0,GAMEPAD_BUTTON_LEFT_FACE_DOWN);
        case ACTION_LEFT:  return IsKeyReleased(KEY_A) || IsKeyReleased(KEY_LEFT) || IsGamepadButtonReleased(0,GAMEPAD_BUTTON_LEFT_FACE_LEFT);
        case ACTION_RIGHT: return IsKeyReleased(KEY_D) || IsKeyReleased(KEY_RIGHT) || IsGamepadButtonReleased(0,GAMEPAD_BUTTON_LEFT_FACE_RIGHT);
        case ACTION_FIRE:  return IsKeyReleased(KEY_SPACE) || IsGamepadButtonReleased(0,GAMEPAD_BUTTON_RIGHT_FACE_DOWN);
        default: return false;
    }
}

bool InputActionDown(const GameContext *ctx, ActionType action)
{
    (void)ctx;
    switch (action) {
        case ACTION_UP:    return IsKeyDown(KEY_W) || IsKeyDown(KEY_UP) || IsGamepadButtonDown(0,GAMEPAD_BUTTON_LEFT_FACE_UP);
        case ACTION_DOWN:  return IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN) || IsGamepadButtonDown(0,GAMEPAD_BUTTON_LEFT_FACE_DOWN);
        case ACTION_LEFT:  return IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT) || IsGamepadButtonDown(0,GAMEPAD_BUTTON_LEFT_FACE_LEFT);
        case ACTION_RIGHT: return IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT) || IsGamepadButtonDown(0,GAMEPAD_BUTTON_LEFT_FACE_RIGHT);
        case ACTION_FIRE:  return IsKeyDown(KEY_SPACE) || IsGamepadButtonDown(0,GAMEPAD_BUTTON_RIGHT_FACE_DOWN);
        default: return false;
    }
}

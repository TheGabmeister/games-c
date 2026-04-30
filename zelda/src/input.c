#include "input.h"
#include "game_config.h"
#include "raylib.h"

bool input_left(void) {
    return IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A) ||
           IsGamepadButtonDown(0, GAMEPAD_BUTTON_LEFT_FACE_LEFT) ||
           GetGamepadAxisMovement(0, GAMEPAD_AXIS_LEFT_X) < -GAMEPAD_DEADZONE;
}

bool input_right(void) {
    return IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D) ||
           IsGamepadButtonDown(0, GAMEPAD_BUTTON_LEFT_FACE_RIGHT) ||
           GetGamepadAxisMovement(0, GAMEPAD_AXIS_LEFT_X) > GAMEPAD_DEADZONE;
}

bool input_up(void) {
    return IsKeyDown(KEY_UP) || IsKeyDown(KEY_W) ||
           IsGamepadButtonDown(0, GAMEPAD_BUTTON_LEFT_FACE_UP) ||
           GetGamepadAxisMovement(0, GAMEPAD_AXIS_LEFT_Y) < -GAMEPAD_DEADZONE;
}

bool input_down(void) {
    return IsKeyDown(KEY_DOWN) || IsKeyDown(KEY_S) ||
           IsGamepadButtonDown(0, GAMEPAD_BUTTON_LEFT_FACE_DOWN) ||
           GetGamepadAxisMovement(0, GAMEPAD_AXIS_LEFT_Y) > GAMEPAD_DEADZONE;
}

bool input_left_pressed(void) {
    return IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_A) ||
           IsGamepadButtonPressed(0, GAMEPAD_BUTTON_LEFT_FACE_LEFT);
}

bool input_right_pressed(void) {
    return IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_D) ||
           IsGamepadButtonPressed(0, GAMEPAD_BUTTON_LEFT_FACE_RIGHT);
}

bool input_up_pressed(void) {
    return IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W) ||
           IsGamepadButtonPressed(0, GAMEPAD_BUTTON_LEFT_FACE_UP);
}

bool input_down_pressed(void) {
    return IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S) ||
           IsGamepadButtonPressed(0, GAMEPAD_BUTTON_LEFT_FACE_DOWN);
}

bool input_attack(void) {
    return IsKeyPressed(KEY_SPACE) ||
           IsGamepadButtonPressed(0, GAMEPAD_BUTTON_RIGHT_FACE_LEFT);
}

bool input_use_item(void) {
    return IsKeyPressed(KEY_LEFT_SHIFT) ||
           IsGamepadButtonPressed(0, GAMEPAD_BUTTON_RIGHT_FACE_UP);
}

bool input_confirm(void) {
    return IsKeyPressed(KEY_ENTER) ||
           IsGamepadButtonPressed(0, GAMEPAD_BUTTON_RIGHT_FACE_DOWN);
}

bool input_back(void) {
    return IsKeyPressed(KEY_ESCAPE) ||
           IsGamepadButtonPressed(0, GAMEPAD_BUTTON_RIGHT_FACE_RIGHT);
}

bool input_pause(void) {
    return IsKeyPressed(KEY_P) ||
           IsGamepadButtonPressed(0, GAMEPAD_BUTTON_MIDDLE_RIGHT);
}

bool input_restart(void) {
    return IsKeyPressed(KEY_R) ||
           IsGamepadButtonPressed(0, GAMEPAD_BUTTON_MIDDLE_LEFT);
}

#include <engine.h>

#include "../components/input.h"
#include "../components/settings.h"
#include "../components/display.h"

#include "input.h"

#include "../defines.h"

//==============================================================================

void process_input(ecs_iter_t *it)
{
  
  Input *input = ecs_singleton_get_mut(it->world, Input);
  Settings *settings = ecs_singleton_get_mut(it->world, Settings);
  Display *display = ecs_singleton_get_mut(it->world, Display);
  *input = (Input){0};

  //input->quit |= WindowShouldClose();
  input->quit |= is_key_pressed(KEY_LEFT_CONTROL) && is_key_pressed(KEY_Q);
  input->quit |= is_key_pressed(KEY_LEFT_CONTROL) && is_key_pressed(KEY_W);
  input->toggle_fullscreen = is_key_pressed(KEY_LEFT_CONTROL) && is_key_pressed(KEY_F);
  input->toggle_pause = is_key_pressed(KEY_P);
/*
  input->joystick.x = 0;
  input->joystick.y = 0;
  settings->gamepad = -1;
  for (int i = 0; i < 4; ++i)
  {
    if (!IsGamepadAvailable(i))
      continue;
    if (settings->gamepad < 0)
      settings->gamepad = i;
    input->quit |= IsGamepadButtonPressed(i, GAMEPAD_BUTTON_MIDDLE_LEFT);
    input->select |= IsGamepadButtonPressed(i, GAMEPAD_BUTTON_RIGHT_FACE_DOWN);
    input->select |= IsGamepadButtonPressed(i, GAMEPAD_BUTTON_RIGHT_FACE_RIGHT);
    input->toggle_pause |= IsGamepadButtonPressed(i, GAMEPAD_BUTTON_MIDDLE_RIGHT);
    input->joystick.x += GetGamepadAxisMovement(i, GAMEPAD_AXIS_LEFT_X);
    input->joystick.y += GetGamepadAxisMovement(i, GAMEPAD_AXIS_LEFT_Y);
    if (IsGamepadButtonDown(i, GAMEPAD_BUTTON_LEFT_FACE_LEFT))
      input->joystick.x += -1;
    if (IsGamepadButtonDown(i, GAMEPAD_BUTTON_LEFT_FACE_RIGHT))
      input->joystick.x += 1;
    if (IsGamepadButtonDown(i, GAMEPAD_BUTTON_LEFT_FACE_UP))
      input->joystick.y += -1;
    if (IsGamepadButtonDown(i, GAMEPAD_BUTTON_LEFT_FACE_DOWN))
      input->joystick.y += 1;
  }
*/
  input->drag |= IsMouseButtonDown(MOUSE_LEFT_BUTTON);
  input->select |= IsMouseButtonPressed(MOUSE_LEFT_BUTTON);
  input->pointer = Vector2Scale(Vector2Subtract(GetMousePosition(), (vector2){display->screen.x, display->screen.y}), 1.0 / display->scale);
  input->wheel = GetMouseWheelMove();

  input->select |= is_key_pressed(KEY_ENTER);
  input->select |= is_key_pressed(KEY_SPACE);
  if (is_key_pressed(KEY_A) || is_key_pressed(KEY_J) || is_key_pressed(KEY_LEFT) || is_key_pressed(KEY_KP_4))
    input->joystick.x += -1;
  if (is_key_pressed(KEY_D) || is_key_pressed(KEY_L) || is_key_pressed(KEY_RIGHT) || is_key_pressed(KEY_KP_6))
    input->joystick.x += 1;
  if (is_key_pressed(KEY_W) || is_key_pressed(KEY_I) || is_key_pressed(KEY_UP) || is_key_pressed(KEY_KP_8))
    input->joystick.y += -1;
  if (is_key_pressed(KEY_S) || is_key_pressed(KEY_K) || is_key_pressed(KEY_DOWN) || is_key_pressed(KEY_KP_2))
    input->joystick.y += 1;

  // input->joystick.x = Clamp(input->joystick.x, -1.0, 1.0);
  // input->joystick.y = Clamp(input->joystick.y, -1.0, 1.0);
  // if (Vector2LengthSqr(input->joystick) < 0.3)
  //   input->joystick = Vector2Zero();

#ifdef DEBUG
  input->toggle_debug = is_key_pressed(KEY_TAB);
#endif

}

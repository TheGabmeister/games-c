#ifndef INPUT_COMPONENT_H
#define INPUT_COMPONENT_H

#include <flecs.h>
#include <engine.h>

typedef struct Input
{
  bool quit;
  bool select;
  bool drag;
  bool toggle_pause;
  bool toggle_debug;
  bool toggle_fullscreen;
  vector2 joystick;
  vector2 pointer;
  float wheel;
} Input;

ECS_COMPONENT_DECLARE(Input);

#endif

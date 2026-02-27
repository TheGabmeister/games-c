#ifndef GAME_H
#define GAME_H

#include "managers/asset.h"
#include "systems/renderer.h"
#include "systems/input.h"
#include "systems/movement.h"
#include "components/velocity.h"
#include "settings.h"

static SDL_Window   *window;
static SDL_Renderer *renderer;
static ecs_world_t  *world;

bool isRunning;

void game_init();
void game_run();
void game_destroy();

void setup_window();
void load_level();
void spawn_entities();

#endif
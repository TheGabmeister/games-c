#ifndef GAME_H
#define GAME_H

#include "sarsa.h"
#include "asset_registry.h"
#include "systems/renderer.h"
#include "systems/input.h"
#include "systems/movement.h"

#define WINDOW_TITLE    "Space Invaders"
#define WINDOW_WIDTH    800
#define WINDOW_HEIGHT   600

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
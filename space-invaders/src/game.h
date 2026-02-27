#ifndef GAME_H
#define GAME_H

#include "sarsa.h"
#include "asset_registry.h"
#include "components/transform.h"
#include "components/sprite.h"

#define WINDOW_TITLE    "Space Invaders"
#define WINDOW_WIDTH    800
#define WINDOW_HEIGHT   600

static SDL_Window   *window;
static SDL_Renderer *renderer;
static ecs_world_t  *world;
static ecs_query_t  *render_query;

void game_init();
void game_run();
void game_destroy();

void setup_window();
void load_level();
void spawn_entites();

#endif
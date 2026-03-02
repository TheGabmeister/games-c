#ifndef GAME_H
#define GAME_H

#include "defines.h"
#include "texture.h"
#include "managers/asset.h"
#include "managers/prefab.h"
#include "managers/component.h"
#include "systems/renderer.h"
#include "systems/input.h"
#include "systems/movement.h"
#include "components/health.h"
#include "components/box_collider.h"
#include "components/velocity.h"
#include "settings.h"

SDL_Window   *window;
SDL_Renderer *renderer;
ecs_world_t  *world;
ecs_entity_t Player;
ecs_entity_t Enemy;
ecs_entity_t Projectile;

bool isRunning;

void game_init();
void game_run();
void game_destroy();

void setup_window();
void load_level();

#endif
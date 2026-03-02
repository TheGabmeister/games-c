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
#include "systems/combat.h"
#include "systems/boundary.h"
#include "systems/collision.h"
#include "systems/gui.h"
#include "components/health.h"
#include "components/box_collider.h"
#include "components/velocity.h"
#include "settings.h"

extern SDL_Window   *window;
extern SDL_Renderer *renderer;
extern ecs_world_t  *world;
extern ecs_entity_t  Player;
extern ecs_entity_t  Enemy;
extern ecs_entity_t  Projectile;
extern ecs_entity_t  Shooting;

extern bool isRunning;

void game_init();
void game_run();
void game_destroy();

void setup_window();
void load_level();

#endif
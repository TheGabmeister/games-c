#include <flecs.h>
#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>

#include "game.h"

static ecs_world_t *_world = NULL;

void game_init(void)
{
  _world = ecs_init();
}

//------------------------------------------------------------------------------

void game_loop(void)
{

}

//------------------------------------------------------------------------------

void game_fini(void)
{
    ecs_fini(_world);
    _world = NULL;
}

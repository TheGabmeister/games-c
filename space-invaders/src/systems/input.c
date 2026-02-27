#include "input.h"
#include <SDL3/SDL.h>
#include "tags.h"
#include "../components/velocity.h"
#include "settings.h"

static ecs_query_t *input_query;

void input_system_init(ecs_world_t *world)
{
    input_query = ecs_query(world, {
        .terms = {
            { .id = Player },
            { .id = ecs_id(Velocity) }
        }
    });
}

void input_system_run(ecs_world_t *world)
{
    const bool *keys = SDL_GetKeyboardState(NULL);

    float dx = 0.0f, dy = 0.0f;
    if (keys[SDL_SCANCODE_A] || keys[SDL_SCANCODE_LEFT])  dx -= PLAYER_SPEED;
    if (keys[SDL_SCANCODE_D] || keys[SDL_SCANCODE_RIGHT]) dx += PLAYER_SPEED;
    if (keys[SDL_SCANCODE_W] || keys[SDL_SCANCODE_UP])    dy -= PLAYER_SPEED;
    if (keys[SDL_SCANCODE_S] || keys[SDL_SCANCODE_DOWN])  dy += PLAYER_SPEED;

    ecs_iter_t it = ecs_query_iter(world, input_query);
    while (ecs_query_next(&it))
    {
        Velocity *velocities = ecs_field(&it, Velocity, 1);
        for (int i = 0; i < it.count; i++)
        {
            velocities[i].x = dx;
            velocities[i].y = dy;
        }
    }
}

void input_system_destroy(void)
{
    ecs_query_fini(input_query);
}

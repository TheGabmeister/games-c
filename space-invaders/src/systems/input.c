#include "input.h"
#include <SDL3/SDL.h>
#include "tags.h"
#include "../components/velocity.h"
#include "../components/transform.h"
#include "../managers/prefab.h"
#include "settings.h"

static ecs_query_t *input_query;
static ecs_query_t *shoot_query;
static bool         prev_space = false;

void input_system_init(ecs_world_t *world)
{
    input_query = ecs_query(world, {
        .terms = {
            { .id = Player },
            { .id = ecs_id(Velocity) }
        }
    });

    shoot_query = ecs_query(world, {
        .terms = {
            { .id = Player },
            { .id = ecs_id(Transform) }
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

    bool cur_space = keys[SDL_SCANCODE_SPACE];
    if (cur_space && !prev_space)
    {
        ecs_iter_t sit = ecs_query_iter(world, shoot_query);
        while (ecs_query_next(&sit))
        {
            Transform *transforms = ecs_field(&sit, Transform, 1);
            for (int i = 0; i < sit.count; i++)
            {
                prefab_spawn_projectile(world,
                    transforms[i].position[0],
                    transforms[i].position[1]);
            }
        }
    }
    prev_space = cur_space;
}

void input_system_destroy(void)
{
    ecs_query_fini(input_query);
    ecs_query_fini(shoot_query);
}

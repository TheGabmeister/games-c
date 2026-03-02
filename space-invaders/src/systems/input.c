#include "input.h"
#include <SDL3/SDL.h>
#include "tags.h"
#include "../components/velocity.h"
#include "../components/transform.h"
#include "../components/box_collider.h"
#include "settings.h"

/* Terms: [Player(tag), Velocity, Transform, BoxCollider]
 * indices:     0           1         2           3       */
static ecs_query_t *input_query;
static ecs_query_t *shoot_query;
static bool         prev_space = false;

void input_system_init(ecs_world_t *world)
{
    input_query = ecs_query(world, {
        .terms = {
            { .id = Player },
            { .id = ecs_id(Velocity) },
            { .id = ecs_id(Transform) },
            { .id = ecs_id(BoxCollider) }
        }
    });

    shoot_query = ecs_query(world, {
        .terms = {
            { .id = Player }
        }
    });
}

void input_system_run(ecs_world_t *world)
{
    const bool *keys = SDL_GetKeyboardState(NULL);

    float dx = 0.0f;
    if (keys[SDL_SCANCODE_A] || keys[SDL_SCANCODE_LEFT])  dx -= PLAYER_SPEED;
    if (keys[SDL_SCANCODE_D] || keys[SDL_SCANCODE_RIGHT]) dx += PLAYER_SPEED;

    ecs_iter_t it = ecs_query_iter(world, input_query);
    while (ecs_query_next(&it))
    {
        Velocity    *velocities = ecs_field(&it, Velocity,    1);
        Transform   *transforms = ecs_field(&it, Transform,   2);
        BoxCollider *colliders  = ecs_field(&it, BoxCollider, 3);
        for (int i = 0; i < it.count; i++)
        {
            float hw = colliders[i].w * 0.5f;
            float x  = transforms[i].position[0];

            /* Hard-clamp position to fix any prior-frame overshoot */
            if (x - hw < 0.0f)                    transforms[i].position[0] = hw;
            if (x + hw > (float)WINDOW_WIDTH)      transforms[i].position[0] = (float)WINDOW_WIDTH - hw;

            /* Suppress velocity that would push the player further out-of-bounds */
            float vx = dx;
            if (x - hw <= 0.0f && vx < 0.0f)                vx = 0.0f;
            if (x + hw >= (float)WINDOW_WIDTH && vx > 0.0f) vx = 0.0f;

            velocities[i].x = vx;
            velocities[i].y = 0.0f;
        }
    }

    bool cur_space = keys[SDL_SCANCODE_SPACE];
    if (cur_space && !prev_space)
    {
        ecs_iter_t sit = ecs_query_iter(world, shoot_query);
        while (ecs_query_next(&sit))
        {
            for (int i = 0; i < sit.count; i++)
                ecs_add_id(world, sit.entities[i], Shooting);
        }
    }
    prev_space = cur_space;
}

void input_system_destroy(void)
{
    ecs_query_fini(input_query);
    ecs_query_fini(shoot_query);
}

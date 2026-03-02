#include "renderer.h"
#include "../components/box_collider.h"

static ecs_query_t *render_query;

void renderer_system_init(ecs_world_t *world)
{
    render_query = ecs_query(world, {
        .terms = {
            { ecs_id(Transform) },
            { ecs_id(Sprite) },
            { .id = ecs_id(BoxCollider), .oper = EcsOptional }
        }
    });
}

void renderer_system_run(ecs_world_t *world, SDL_Renderer *sdl_renderer)
{
    ecs_iter_t it = ecs_query_iter(world, render_query);
    while (ecs_query_next(&it))
    {
        Transform   *transforms = ecs_field(&it, Transform,   0);
        Sprite      *sprites    = ecs_field(&it, Sprite,      1);
        bool has_collider = ecs_field_is_set(&it, 2);
        BoxCollider *colliders  = has_collider ? ecs_field(&it, BoxCollider, 2) : NULL;

        for (int i = 0; i < it.count; i++)
        {
            if (sprites[i].texture)
            {
                float tw, th;
                SDL_GetTextureSize(sprites[i].texture, &tw, &th);
                SDL_FRect dst = {
                    transforms[i].position[0] - tw * transforms[i].scale[0] * 0.5f,
                    transforms[i].position[1] - th * transforms[i].scale[1] * 0.5f,
                    tw * transforms[i].scale[0],
                    th * transforms[i].scale[1]
                };
                SDL_RenderTexture(sdl_renderer, sprites[i].texture, NULL, &dst);
            }
            else if (colliders)
            {
                SDL_Color c = sprites[i].color;
                SDL_SetRenderDrawColor(sdl_renderer, c.r, c.g, c.b, c.a);
                SDL_FRect rect = {
                    transforms[i].position[0] - colliders[i].w * 0.5f,
                    transforms[i].position[1] - colliders[i].h * 0.5f,
                    colliders[i].w,
                    colliders[i].h
                };
                SDL_RenderFillRect(sdl_renderer, &rect);
                SDL_SetRenderDrawColor(sdl_renderer, 0, 0, 0, 255);
            }
        }
    }
}

void renderer_system_destroy(void)
{
    ecs_query_fini(render_query);
}

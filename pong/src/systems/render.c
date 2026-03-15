#include "../defines.h"

#include "../components/transform.h"
#include "../components/sprite.h"
#include "../components/shape.h"
#include "../components/collider.h"
#include "../components/label.h"

#include "../managers/system.h"

#include <SDL3/SDL.h>
#include "../platform.h"

#include "render.h"

//==============================================================================
// Helpers
//==============================================================================

static inline SDL_FRect _sdl_rect(SDL_FRect r)
{
  return (SDL_FRect){r.x, r.y, r.w, r.h};
}

static inline void _set_tint(SDL_Texture *tex, SDL_Color c)
{
    SDL_SetTextureColorMod(tex, c.r, c.g, c.b);
    SDL_SetTextureAlphaMod(tex, c.a);
}


static bool _show_colliders = false;

void toggle_show_colliders(void)
{
    _show_colliders = !_show_colliders;
}

void render_clear(ecs_iter_t *it)
{
    SDL_Renderer *renderer = get_renderer();
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
}

void render_present(ecs_iter_t *it)
{
    SDL_RenderPresent(get_renderer());
}

//==============================================================================

void render_shapes(ecs_iter_t *it)
{
    Shape *shape = ecs_field(it, Shape, 0);
    Transform *spatial = ecs_field(it, Transform, 1);
    SDL_Renderer *renderer = get_renderer();
    for (int i = 0; i < it->count; ++i)
    {
        SDL_SetRenderDrawColor(renderer, shape[i].color.r, shape[i].color.g, shape[i].color.b, shape[i].color.a);
        switch (shape[i].type)
        {
            case SHAPE_RECTANGLE:
            {
                float w = shape[i].rectangle.width;
                float h = shape[i].rectangle.height;
                SDL_FRect dst = {spatial[i].position.x - w * 0.5f, spatial[i].position.y - h * 0.5f, w, h};
                SDL_RenderFillRect(renderer, &dst);
                break;
            }
            case SHAPE_CIRCLE:
            {
                float r = shape[i].circle.radius;
                float cx = spatial[i].position.x;
                float cy = spatial[i].position.y;
                for (float dy = -r; dy <= r; dy += 1.0f)
                {
                    float dx = SDL_sqrtf(r * r - dy * dy);
                    SDL_RenderLine(renderer, cx - dx, cy + dy, cx + dx, cy + dy);
                }
                break;
            }
            case SHAPE_TRIANGLE:
            {
                float base = shape[i].triangle.base;
                float h = shape[i].triangle.height;
                float cx = spatial[i].position.x;
                float cy = spatial[i].position.y;
                float x1 = cx, y1 = cy - h * 0.5f;
                float x2 = cx - base * 0.5f, y2 = cy + h * 0.5f;
                float x3 = cx + base * 0.5f, y3 = cy + h * 0.5f;
                SDL_RenderLine(renderer, x1, y1, x2, y2);
                SDL_RenderLine(renderer, x2, y2, x3, y3);
                SDL_RenderLine(renderer, x3, y3, x1, y1);
                break;
            }
        }
    }
}

//==============================================================================

void render_colliders(ecs_iter_t *it)
{
    if (!_show_colliders) return;

    Collider  *collider  = ecs_field(it, Collider,  0);
    Transform *transform = ecs_field(it, Transform, 1);
    SDL_Renderer *renderer = get_renderer();
    SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);

    for (int i = 0; i < it->count; ++i)
    {
        float cx = transform[i].position.x;
        float cy = transform[i].position.y;

        switch (collider[i].type)
        {
            case COLLIDER_RECT:
            {
                float hw = collider[i].rect.width  * 0.5f;
                float hh = collider[i].rect.height * 0.5f;
                SDL_FRect dst = {cx - hw, cy - hh, collider[i].rect.width, collider[i].rect.height};
                SDL_RenderRect(renderer, &dst);
                break;
            }
            case COLLIDER_CIRCLE:
            {
                float r = collider[i].circle.radius;
                for (float dy = -r; dy <= r; dy += 1.0f)
                {
                    float dx = SDL_sqrtf(r * r - dy * dy);
                    SDL_RenderLine(renderer, cx - dx, cy + dy, cx + dx, cy + dy);
                }
                break;
            }
        }
    }
}

//==============================================================================

void render_labels(ecs_iter_t *it)
{
    Label     *label     = ecs_field(it, Label,     0);
    Transform *transform = ecs_field(it, Transform, 1);
    SDL_Renderer *renderer = get_renderer();

    for (int i = 0; i < it->count; ++i)
    {
        SDL_SetRenderDrawColor(renderer, label[i].color.r, label[i].color.g,
                               label[i].color.b, label[i].color.a);
        float s = label[i].scale;
        if (s <= 0.0f) s = 1.0f;
        SDL_SetRenderScale(renderer, s, s);
        float x = transform[i].position.x / s;
        float y = transform[i].position.y / s;
        SDL_RenderDebugText(renderer, x, y, label[i].text);
        SDL_SetRenderScale(renderer, 1.0f, 1.0f);
    }
}

//==============================================================================

void render_sprites(ecs_iter_t *it)
{
    Sprite *sprite = ecs_field(it, Sprite, 0);
    Transform *spatial = ecs_field(it, Transform, 1);
    SDL_Renderer *renderer = get_renderer();
    for (int i = 0; i < it->count; ++i)
    {
      SDL_FRect src = _sdl_rect(sprite[i].src);
      float w = sprite[i].scale * sprite[i].src.w;
      float h = sprite[i].scale * sprite[i].src.h;
      SDL_FRect dst = {spatial[i].position.x - w * 0.5f, spatial[i].position.y - h * 0.5f, w, h};
      SDL_RenderTextureRotated(renderer, sprite[i].texture, &src, &dst, spatial[i].rotation, NULL, SDL_FLIP_NONE);
    }
}

#include "../components/player.h"
#include "../components/transform.h"
#include "../components/room_occupant.h"
#include "../data/rooms.h"
#include "../defines.h"
#include "../game.h"
#include "../platform.h"
#include "fog.h"
#include <math.h>

static SDL_Texture *fog_texture = NULL;
static ecs_query_t *player_query = NULL;

//==============================================================================
// Init / Fini
//==============================================================================

void fog_init(void)
{
    SDL_Renderer *renderer = get_renderer();
    if (!renderer)
    {
        SDL_Log("fog: renderer unavailable during initialization");
        return;
    }

    fog_texture = SDL_CreateTexture(
        renderer,
        SDL_PIXELFORMAT_RGBA8888,
        SDL_TEXTUREACCESS_TARGET,
        PLAYFIELD_W, PLAYFIELD_H);

    if (!fog_texture)
    {
        SDL_Log("fog: failed to create render texture: %s", SDL_GetError());
        return;
    }

    SDL_SetTextureBlendMode(fog_texture, SDL_BLENDMODE_BLEND);
}

void fog_fini(void)
{
    if (fog_texture)
    {
        SDL_DestroyTexture(fog_texture);
        fog_texture = NULL;
    }
}

//==============================================================================
// System callback — 0-term system (no query)
//==============================================================================

void render_fog(ecs_iter_t *it)
{
    (void)it;
    if (!fog_texture)
        return;

    /* Only render fog in dark rooms */
    GameState *gs = game_state_get();
    const RoomDef *room = room_get(gs->current_room);
    if (!room || !(room->flags & ROOM_FLAG_DARK))
        return;

    /* Find the player position */
    ecs_world_t *world = game_world_get();
    if (!world)
        return;

    if (!player_query)
    {
        player_query = ecs_query(world, {
            .terms = {
                { .id = ecs_id(Player) },
                { .id = ecs_id(Transform) }
            }
        });
    }

    float px = PLAYFIELD_W * 0.5f;
    float py = PLAYFIELD_H * 0.5f;

    ecs_iter_t qit = ecs_query_iter(world, player_query);
    while (ecs_query_next(&qit))
    {
        Transform *t = ecs_field(&qit, Transform, 1);
        for (int i = 0; i < qit.count; ++i)
        {
            px = t[i].position.x;
            py = t[i].position.y;
        }
    }

    /* Draw fog overlay onto target texture */
    SDL_Renderer *renderer = get_renderer();
    SDL_SetRenderTarget(renderer, fog_texture);

    /* Fill entire texture with opaque black */
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    /* Punch a transparent circular hole at the player position */
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_NONE);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);

    float r = 120.0f;
    for (float dy = -r; dy <= r; dy += 1.0f)
    {
        float dx = SDL_sqrtf(r * r - dy * dy);
        SDL_RenderLine(renderer, px - dx, py + dy, px + dx, py + dy);
    }

    /* Soft gradient ring (semi-transparent) from radius 120 to 160 */
    float outer = 160.0f;
    for (float dy = -outer; dy <= outer; dy += 1.0f)
    {
        float dx_outer = SDL_sqrtf(outer * outer - dy * dy);
        float dx_inner_sq = r * r - dy * dy;

        if (dx_inner_sq < 0.0f)
        {
            /* Entire row is in the gradient band */
            float dist = SDL_sqrtf(dy * dy);
            float t = (dist - r) / (outer - r);
            t = CLAMP(t, 0.0f, 1.0f);
            Uint8 a = (Uint8)(t * 255.0f);
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, a);
            SDL_RenderLine(renderer, px - dx_outer, py + dy, px + dx_outer, py + dy);
        }
        else
        {
            float dx_inner = SDL_sqrtf(dx_inner_sq);
            /* Left gradient band */
            for (float gx = dx_inner; gx <= dx_outer; gx += 1.0f)
            {
                float dist = SDL_sqrtf(gx * gx + dy * dy);
                float t = (dist - r) / (outer - r);
                t = CLAMP(t, 0.0f, 1.0f);
                Uint8 a = (Uint8)(t * 255.0f);
                SDL_SetRenderDrawColor(renderer, 0, 0, 0, a);
                SDL_RenderPoint(renderer, px - gx, py + dy);
                SDL_RenderPoint(renderer, px + gx, py + dy);
            }
        }
    }

    /* Restore render state */
    SDL_SetRenderTarget(renderer, NULL);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

    /* Draw fog texture over the playfield */
    SDL_FRect dst = { PLAYFIELD_X, PLAYFIELD_Y, PLAYFIELD_W, PLAYFIELD_H };
    SDL_RenderTexture(renderer, fog_texture, NULL, &dst);
}

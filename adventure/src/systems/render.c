#include "../defines.h"
#include "../components/transform.h"
#include "../components/shape.h"
#include "../components/collider.h"
#include "../components/room_occupant.h"
#include "../components/player.h"
#include "../components/dragon.h"
#include "../components/bat.h"
#include "../components/item.h"
#include "../components/carried.h"
#include "../components/gate.h"
#include "../components/glow.h"
#include "../data/rooms.h"
#include "../game.h"
#include "../platform.h"

#include <SDL3/SDL.h>
#include "render.h"

/*=============================================================================
 * Helpers
 *===========================================================================*/
static void _fill_rect(SDL_Renderer *r, float x, float y, float w, float h)
{
    SDL_FRect rect = { x, y, w, h };
    SDL_RenderFillRect(r, &rect);
}

static void _outline_rect(SDL_Renderer *r, float x, float y, float w, float h)
{
    SDL_FRect rect = { x, y, w, h };
    SDL_RenderRect(r, &rect);
}

/*=============================================================================
 * render_clear
 *===========================================================================*/
void render_clear(ecs_iter_t *it)
{
    (void)it;
    SDL_Renderer *r = get_renderer();
    SDL_SetRenderDrawColor(r, 0, 0, 0, 255);
    SDL_RenderClear(r);
}

/*=============================================================================
 * render_room — draw walls, gates, and room geometry
 *===========================================================================*/
void render_room(ecs_iter_t *it)
{
    (void)it;
    SDL_Renderer *r = get_renderer();
    GameState *gs = game_state_get();
    const RoomDef *room = room_get(gs->current_room);
    if (!room) return;

    float ox = (float)PLAYFIELD_X;
    float oy = (float)PLAYFIELD_Y;

    /* Room background tint */
    SDL_SetRenderDrawColor(r, room->ambient_tint.r, room->ambient_tint.g,
                           room->ambient_tint.b, 255);
    _fill_rect(r, ox, oy, (float)PLAYFIELD_W, (float)PLAYFIELD_H);

    /* Wall color */
    SDL_SetRenderDrawColor(r, 80, 80, 80, 255);

    /* Boundary walls with exit gaps */
    float wt = WALL_THICKNESS;
    float pw = (float)PLAYFIELD_W;
    float ph = (float)PLAYFIELD_H;

    /* North wall */
    {
        const RoomExit *ex = &room->exits[DIR_NORTH];
        if (ex->destination_room == ROOM_NONE) {
            _fill_rect(r, ox, oy, pw, wt);
        } else {
            /* Left segment */
            if (ex->span_min > 0)
                _fill_rect(r, ox, oy, ex->span_min, wt);
            /* Right segment */
            if (ex->span_max < pw)
                _fill_rect(r, ox + ex->span_max, oy, pw - ex->span_max, wt);
        }
    }

    /* South wall */
    {
        const RoomExit *ex = &room->exits[DIR_SOUTH];
        if (ex->destination_room == ROOM_NONE) {
            _fill_rect(r, ox, oy + ph - wt, pw, wt);
        } else {
            if (ex->span_min > 0)
                _fill_rect(r, ox, oy + ph - wt, ex->span_min, wt);
            if (ex->span_max < pw)
                _fill_rect(r, ox + ex->span_max, oy + ph - wt, pw - ex->span_max, wt);
        }
    }

    /* West wall */
    {
        const RoomExit *ex = &room->exits[DIR_WEST];
        if (ex->destination_room == ROOM_NONE) {
            _fill_rect(r, ox, oy, wt, ph);
        } else {
            if (ex->span_min > 0)
                _fill_rect(r, ox, oy, wt, ex->span_min);
            if (ex->span_max < ph)
                _fill_rect(r, ox, oy + ex->span_max, wt, ph - ex->span_max);
        }
    }

    /* East wall */
    {
        const RoomExit *ex = &room->exits[DIR_EAST];
        if (ex->destination_room == ROOM_NONE) {
            _fill_rect(r, ox + pw - wt, oy, wt, ph);
        } else {
            if (ex->span_min > 0)
                _fill_rect(r, ox + pw - wt, oy, wt, ex->span_min);
            if (ex->span_max < ph)
                _fill_rect(r, ox + pw - wt, oy + ex->span_max, wt, ph - ex->span_max);
        }
    }

    /* Wall edge accents (lighter) */
    SDL_SetRenderDrawColor(r, 110, 110, 110, 255);

    /* Interior walls */
    SDL_SetRenderDrawColor(r, 80, 80, 80, 255);
    for (int i = 0; i < room->interior_wall_count; i++)
    {
        const WallRect *w = &room->interior_walls[i];
        _fill_rect(r, ox + w->x, oy + w->y, w->w, w->h);
    }
    /* Interior wall edge accents */
    SDL_SetRenderDrawColor(r, 110, 110, 110, 255);
    for (int i = 0; i < room->interior_wall_count; i++)
    {
        const WallRect *w = &room->interior_walls[i];
        _outline_rect(r, ox + w->x, oy + w->y, w->w, w->h);
    }

    /* Render gates */
    ecs_world_t *world = game_world_get();
    if (world)
    {
        static ecs_query_t *gate_q = NULL;
        if (!gate_q)
        {
            gate_q = ecs_query(world, {
                .terms = {{ ecs_id(Gate), .inout = EcsIn }}
            });
        }

        ecs_iter_t git = ecs_query_iter(world, gate_q);
        while (ecs_query_next(&git))
        {
            Gate *gates = ecs_field(&git, Gate, 0);
            for (int i = 0; i < git.count; i++)
            {
                if (gates[i].room_id != gs->current_room) continue;
                if (gates[i].open) continue;

                /* Draw closed gate */
                SDL_Color gc;
                switch (gates[i].key_type)
                {
                    case KEY_GOLD:  gc = (SDL_Color){200, 170, 0, 255}; break;
                    case KEY_BLACK: gc = (SDL_Color){100, 100, 100, 255}; break;
                    case KEY_WHITE: gc = (SDL_Color){220, 220, 230, 255}; break;
                    default:        gc = (SDL_Color){150, 150, 150, 255}; break;
                }
                SDL_SetRenderDrawColor(r, gc.r, gc.g, gc.b, gc.a);

                const RoomExit *ex = &room->exits[gates[i].direction];
                switch (gates[i].direction)
                {
                    case DIR_NORTH:
                        _fill_rect(r, ox + ex->span_min, oy, ex->span_max - ex->span_min, wt);
                        break;
                    case DIR_SOUTH:
                        _fill_rect(r, ox + ex->span_min, oy + ph - wt, ex->span_max - ex->span_min, wt);
                        break;
                    case DIR_WEST:
                        _fill_rect(r, ox, oy + ex->span_min, wt, ex->span_max - ex->span_min);
                        break;
                    case DIR_EAST:
                        _fill_rect(r, ox + pw - wt, oy + ex->span_min, wt, ex->span_max - ex->span_min);
                        break;
                    default: break;
                }
            }
        }
    }

    /* Room name */
    if (gs->debug_show_room_info && room->name)
    {
        SDL_SetRenderDrawColor(r, 255, 255, 255, 200);
        float scale = 1.5f;
        SDL_SetRenderScale(r, scale, scale);
        SDL_RenderDebugText(r, 10.0f / scale, (ph - 20.0f) / scale, room->name);
        SDL_SetRenderScale(r, 1.0f, 1.0f);
    }
}

/*=============================================================================
 * Entity rendering helpers
 *===========================================================================*/

static void _draw_glow(SDL_Renderer *r, float cx, float cy, SDL_Color color, int layers, float radius)
{
    for (int l = layers; l >= 1; l--)
    {
        float t = (float)l / (float)layers;
        float lr = radius * t;
        Uint8 alpha = (Uint8)(30.0f * (1.0f - t));
        SDL_SetRenderDrawColor(r, color.r, color.g, color.b, alpha);
        SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_ADD);
        _fill_rect(r, cx - lr, cy - lr, lr * 2.0f, lr * 2.0f);
    }
    SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND);
}

static void _draw_player(SDL_Renderer *r, float cx, float cy)
{
    /* Glow */
    _draw_glow(r, cx, cy, (SDL_Color){255, 215, 0, 255}, 3, 20.0f);
    /* Core */
    SDL_SetRenderDrawColor(r, 255, 215, 0, 255);
    _fill_rect(r, cx - 8, cy - 8, 16, 16);
    /* Bright core */
    SDL_SetRenderDrawColor(r, 255, 240, 100, 255);
    _fill_rect(r, cx - 4, cy - 4, 8, 8);
}

static void _draw_dragon(SDL_Renderer *r, float cx, float cy, DragonType type)
{
    SDL_Color body_color;
    float bw, bh;
    switch (type)
    {
        case DRAGON_YORGLE:
            body_color = (SDL_Color){180, 200, 0, 255};
            bw = 28; bh = 20;
            break;
        case DRAGON_GRUNDLE:
            body_color = (SDL_Color){0, 180, 0, 255};
            bw = 24; bh = 18;
            break;
        case DRAGON_RHINDLE:
            body_color = (SDL_Color){220, 40, 40, 255};
            bw = 24; bh = 18;
            break;
        default:
            body_color = (SDL_Color){150, 150, 150, 255};
            bw = 24; bh = 18;
            break;
    }

    _draw_glow(r, cx, cy, body_color, 3, 24.0f);

    /* Body */
    SDL_SetRenderDrawColor(r, body_color.r, body_color.g, body_color.b, 255);
    _fill_rect(r, cx - bw * 0.5f, cy - bh * 0.5f, bw, bh);

    /* Head */
    SDL_SetRenderDrawColor(r, body_color.r + 20, body_color.g + 20, body_color.b, 255);
    _fill_rect(r, cx - 5, cy - bh * 0.5f - 6, 10, 6);

    /* Eyes */
    SDL_SetRenderDrawColor(r, 255, 255, 255, 255);
    _fill_rect(r, cx - 4, cy - bh * 0.5f - 5, 2, 2);
    _fill_rect(r, cx + 2, cy - bh * 0.5f - 5, 2, 2);
}

static void _draw_bat(SDL_Renderer *r, float cx, float cy, float time)
{
    /* Body */
    SDL_SetRenderDrawColor(r, 160, 0, 200, 255);
    _fill_rect(r, cx - 6, cy - 5, 12, 10);

    /* Wings - flap animation */
    float flap = SDL_sinf(time * 10.0f) * 4.0f;
    SDL_SetRenderDrawColor(r, 140, 0, 180, 255);
    _fill_rect(r, cx - 18, cy - 3 + flap, 12, 6);
    _fill_rect(r, cx + 6,  cy - 3 - flap, 12, 6);

    /* Eyes */
    SDL_SetRenderDrawColor(r, 255, 255, 0, 255);
    _fill_rect(r, cx - 3, cy - 3, 2, 2);
    _fill_rect(r, cx + 1, cy - 3, 2, 2);
}

static void _draw_item(SDL_Renderer *r, float cx, float cy, ItemType type, float time)
{
    switch (type)
    {
        case ITEM_SWORD:
            _draw_glow(r, cx, cy, (SDL_Color){0, 220, 255, 255}, 2, 16.0f);
            SDL_SetRenderDrawColor(r, 0, 220, 255, 255);
            _fill_rect(r, cx - 3, cy - 14, 6, 28);       /* Blade */
            SDL_SetRenderDrawColor(r, 180, 160, 100, 255);
            _fill_rect(r, cx - 6, cy + 6, 12, 3);         /* Guard */
            SDL_SetRenderDrawColor(r, 140, 100, 40, 255);
            _fill_rect(r, cx - 2, cy + 9, 4, 5);          /* Grip */
            break;

        case ITEM_GOLD_KEY:
            _draw_glow(r, cx, cy, (SDL_Color){200, 170, 0, 255}, 2, 14.0f);
            SDL_SetRenderDrawColor(r, 200, 170, 0, 255);
            _fill_rect(r, cx - 5, cy - 9, 10, 18);        /* Body */
            _fill_rect(r, cx - 8, cy - 9, 16, 5);         /* Top crossbar */
            break;

        case ITEM_BLACK_KEY:
            SDL_SetRenderDrawColor(r, 80, 80, 80, 255);
            _fill_rect(r, cx - 5, cy - 9, 10, 18);
            _fill_rect(r, cx - 8, cy - 9, 16, 5);
            SDL_SetRenderDrawColor(r, 140, 140, 140, 255);
            _outline_rect(r, cx - 5, cy - 9, 10, 18);
            break;

        case ITEM_WHITE_KEY:
            _draw_glow(r, cx, cy, (SDL_Color){220, 220, 230, 255}, 2, 14.0f);
            SDL_SetRenderDrawColor(r, 220, 220, 230, 255);
            _fill_rect(r, cx - 5, cy - 9, 10, 18);
            _fill_rect(r, cx - 8, cy - 9, 16, 5);
            break;

        case ITEM_MAGNET:
            _draw_glow(r, cx, cy, (SDL_Color){220, 140, 0, 255}, 2, 14.0f);
            SDL_SetRenderDrawColor(r, 220, 140, 0, 255);
            /* U-shape: left arm, right arm, bottom bar */
            _fill_rect(r, cx - 8, cy - 8, 4, 16);
            _fill_rect(r, cx + 4, cy - 8, 4, 16);
            _fill_rect(r, cx - 8, cy + 4, 20, 4);
            /* Red/blue tips */
            SDL_SetRenderDrawColor(r, 220, 0, 0, 255);
            _fill_rect(r, cx - 8, cy - 8, 4, 4);
            SDL_SetRenderDrawColor(r, 0, 80, 220, 255);
            _fill_rect(r, cx + 4, cy - 8, 4, 4);
            break;

        case ITEM_BRIDGE:
            SDL_SetRenderDrawColor(r, 180, 130, 50, 255);
            _fill_rect(r, cx - 20, cy - 4, 40, 8);
            /* Planks */
            SDL_SetRenderDrawColor(r, 140, 100, 40, 255);
            for (float bx = cx - 18; bx < cx + 18; bx += 8)
                _fill_rect(r, bx, cy - 4, 1, 8);
            break;

        case ITEM_CHALICE:
        {
            /* Strongest glow / shimmer */
            float shimmer = 0.8f + 0.2f * SDL_sinf(time * 4.0f);
            Uint8 g = (Uint8)(215.0f * shimmer);
            _draw_glow(r, cx, cy, (SDL_Color){255, g, 0, 255}, 4, 28.0f);
            SDL_SetRenderDrawColor(r, 255, 215, 0, 255);
            /* Cup */
            _fill_rect(r, cx - 6, cy - 10, 12, 14);
            /* Base */
            SDL_SetRenderDrawColor(r, 200, 170, 0, 255);
            _fill_rect(r, cx - 8, cy + 4, 16, 4);
            /* Stem */
            _fill_rect(r, cx - 2, cy + 2, 4, 4);
            /* Rim */
            SDL_SetRenderDrawColor(r, 255, 240, 100, 255);
            _fill_rect(r, cx - 7, cy - 10, 14, 2);
            break;
        }

        case ITEM_SECRET_DOT:
            /* Not rendered */
            break;

        default:
            break;
    }
}

/*=============================================================================
 * render_entities — draw all visible entities in the current room
 *===========================================================================*/
void render_entities(ecs_iter_t *it)
{
    (void)it;
    SDL_Renderer *r = get_renderer();
    GameState *gs = game_state_get();
    ecs_world_t *world = game_world_get();
    if (!world) return;

    int cur_room = gs->current_room;
    float ox = (float)PLAYFIELD_X;
    float oy = (float)PLAYFIELD_Y;
    float time = gs->elapsed_time;

    /* Draw items first (below entities) */
    {
        static ecs_query_t *q = NULL;
        if (!q)
        {
            q = ecs_query(world, {
                .terms = {
                    { ecs_id(Item),          .inout = EcsIn },
                    { ecs_id(Transform),     .inout = EcsIn },
                    { ecs_id(RoomOccupant),  .inout = EcsIn },
                }
            });
        }

        ecs_iter_t qit = ecs_query_iter(world, q);
        while (ecs_query_next(&qit))
        {
            Item *items = ecs_field(&qit, Item, 0);
            Transform *transforms = ecs_field(&qit, Transform, 1);
            RoomOccupant *rooms = ecs_field(&qit, RoomOccupant, 2);

            for (int i = 0; i < qit.count; i++)
            {
                if (rooms[i].room_id != cur_room) continue;
                float cx = ox + transforms[i].position.x;
                float cy = oy + transforms[i].position.y;
                _draw_item(r, cx, cy, items[i].type, time);
            }
        }
    }

    /* Draw dragons */
    {
        static ecs_query_t *q = NULL;
        if (!q)
        {
            q = ecs_query(world, {
                .terms = {
                    { ecs_id(Dragon),        .inout = EcsIn },
                    { ecs_id(Transform),     .inout = EcsIn },
                    { ecs_id(RoomOccupant),  .inout = EcsIn },
                }
            });
        }

        ecs_iter_t qit = ecs_query_iter(world, q);
        while (ecs_query_next(&qit))
        {
            Dragon *dragons = ecs_field(&qit, Dragon, 0);
            Transform *transforms = ecs_field(&qit, Transform, 1);
            RoomOccupant *rooms = ecs_field(&qit, RoomOccupant, 2);

            for (int i = 0; i < qit.count; i++)
            {
                if (rooms[i].room_id != cur_room) continue;
                if (dragons[i].state == DRAGON_DEAD) continue;
                float cx = ox + transforms[i].position.x;
                float cy = oy + transforms[i].position.y;
                _draw_dragon(r, cx, cy, dragons[i].type);
            }
        }
    }

    /* Draw bat */
    {
        static ecs_query_t *q = NULL;
        if (!q)
        {
            q = ecs_query(world, {
                .terms = {
                    { ecs_id(Bat),           .inout = EcsIn },
                    { ecs_id(Transform),     .inout = EcsIn },
                    { ecs_id(RoomOccupant),  .inout = EcsIn },
                }
            });
        }

        ecs_iter_t qit = ecs_query_iter(world, q);
        while (ecs_query_next(&qit))
        {
            Transform *transforms = ecs_field(&qit, Transform, 1);
            RoomOccupant *rooms = ecs_field(&qit, RoomOccupant, 2);

            for (int i = 0; i < qit.count; i++)
            {
                if (rooms[i].room_id != cur_room) continue;
                float cx = ox + transforms[i].position.x;
                float cy = oy + transforms[i].position.y;
                _draw_bat(r, cx, cy, time);
            }
        }
    }

    /* Draw player */
    {
        static ecs_query_t *q = NULL;
        if (!q)
        {
            q = ecs_query(world, {
                .terms = {
                    { ecs_id(Player),        .inout = EcsIn },
                    { ecs_id(Transform),     .inout = EcsIn },
                    { ecs_id(RoomOccupant),  .inout = EcsIn },
                }
            });
        }

        ecs_iter_t qit = ecs_query_iter(world, q);
        while (ecs_query_next(&qit))
        {
            Transform *transforms = ecs_field(&qit, Transform, 1);
            RoomOccupant *rooms = ecs_field(&qit, RoomOccupant, 2);

            for (int i = 0; i < qit.count; i++)
            {
                if (rooms[i].room_id != cur_room) continue;
                float cx = ox + transforms[i].position.x;
                float cy = oy + transforms[i].position.y;
                _draw_player(r, cx, cy);
            }
        }
    }
}

/*=============================================================================
 * render_colliders — debug overlay
 *===========================================================================*/
void render_colliders(ecs_iter_t *it)
{
    GameState *gs = game_state_get();
    if (!gs->debug_show_colliders) return;

    Collider  *collider  = ecs_field(it, Collider,  0);
    Transform *transform = ecs_field(it, Transform, 1);
    RoomOccupant *room   = ecs_field(it, RoomOccupant, 2);
    SDL_Renderer *r = get_renderer();

    float ox = (float)PLAYFIELD_X;
    float oy = (float)PLAYFIELD_Y;

    SDL_SetRenderDrawColor(r, 0, 255, 0, 255);

    for (int i = 0; i < it->count; ++i)
    {
        if (room[i].room_id != gs->current_room) continue;

        float cx = ox + transform[i].position.x;
        float cy = oy + transform[i].position.y;

        if (collider[i].type == COLLIDER_RECT)
        {
            float hw = collider[i].rect.width  * 0.5f;
            float hh = collider[i].rect.height * 0.5f;
            _outline_rect(r, cx - hw, cy - hh, collider[i].rect.width, collider[i].rect.height);
        }
        else if (collider[i].type == COLLIDER_CIRCLE)
        {
            float rad = collider[i].circle.radius;
            for (float dy = -rad; dy <= rad; dy += 1.0f)
            {
                float dx = SDL_sqrtf(rad * rad - dy * dy);
                SDL_RenderLine(r, cx - dx, cy + dy, cx + dx, cy + dy);
            }
        }
    }
}

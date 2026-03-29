#include "../components/player.h"
#include "../components/transform.h"
#include "../components/velocity.h"
#include "../components/room_occupant.h"
#include "../components/carried.h"
#include "../components/collider.h"
#include "../components/input.h"
#include "../data/rooms.h"
#include "../defines.h"
#include "../game.h"
#include "player.h"
#include <math.h>

/*=============================================================================
 * player_intent
 * Query: [in] Player, [inout] Transform, [inout] Velocity, [in] RoomOccupant
 *   idx:   0            1                  2                 3
 *===========================================================================*/
void player_intent(ecs_iter_t *it)
{
    if (game_state_get()->scene != SCENE_PLAYING) return;

    Player        *player   = ecs_field(it, Player, 0);
    Transform     *tf       = ecs_field(it, Transform, 1);
    Velocity      *vel      = ecs_field(it, Velocity, 2);
    RoomOccupant  *occ      = ecs_field(it, RoomOccupant, 3);
    const Input   *input    = ecs_singleton_get(it->world, Input);
    if (!input) return;

    (void)player;
    (void)tf;
    (void)occ;

    for (int i = 0; i < it->count; ++i)
    {
        float vx = input->move_x;
        float vy = input->move_y;

        /* Normalize diagonal movement */
        if (vx != 0.0f && vy != 0.0f)
        {
            vx *= 0.7071f;
            vy *= 0.7071f;
        }

        vel[i].value.x = vx * PLAYER_SPEED;
        vel[i].value.y = vy * PLAYER_SPEED;
    }
}

/*=============================================================================
 * carry_sync
 * Query: [in] Carried, [inout] Transform, [inout] RoomOccupant
 *   idx:   0            1                   2
 *===========================================================================*/
void carry_sync(ecs_iter_t *it)
{
    Carried      *carried = ecs_field(it, Carried, 0);
    Transform    *tf      = ecs_field(it, Transform, 1);
    RoomOccupant *occ     = ecs_field(it, RoomOccupant, 2);

    for (int i = 0; i < it->count; ++i)
    {
        ecs_entity_t carrier = carried[i].carrier;

        /* If carrier no longer exists, drop the item */
        if (!ecs_is_alive(it->world, carrier))
        {
            ecs_remove(it->world, it->entities[i], Carried);
            continue;
        }

        const Transform    *carrier_tf  = ecs_get(it->world, carrier, Transform);
        const RoomOccupant *carrier_occ = ecs_get(it->world, carrier, RoomOccupant);

        if (!carrier_tf || !carrier_occ)
        {
            ecs_remove(it->world, it->entities[i], Carried);
            continue;
        }

        tf[i].position.x = carrier_tf->position.x + carried[i].local_offset.x;
        tf[i].position.y = carrier_tf->position.y + carried[i].local_offset.y;
        occ[i].room_id   = carrier_occ->room_id;
    }
}

/*=============================================================================
 * AABB wall collision helper
 *===========================================================================*/
static void resolve_wall_collisions(Transform *tf, const Collider *col,
                                    const RoomDef *room)
{
    float hw = col->rect.width  * 0.5f;
    float hh = col->rect.height * 0.5f;

    for (int w = 0; w < room->interior_wall_count; ++w)
    {
        const WallRect *wall = &room->interior_walls[w];

        float ex = tf->position.x;
        float ey = tf->position.y;

        /* Entity AABB */
        float el = ex - hw;
        float er = ex + hw;
        float et = ey - hh;
        float eb = ey + hh;

        /* Wall AABB */
        float wl = wall->x;
        float wr = wall->x + wall->w;
        float wt = wall->y;
        float wb = wall->y + wall->h;

        /* Check overlap */
        if (el >= wr || er <= wl || et >= wb || eb <= wt)
            continue;

        /* Compute penetration on each axis */
        float pen_right  = er - wl;
        float pen_left   = wr - el;
        float pen_bottom = eb - wt;
        float pen_top    = wb - et;

        /* Find minimum penetration */
        float min_pen = pen_right;
        int axis = 0; /* 0=push left, 1=push right, 2=push up, 3=push down */

        if (pen_left < min_pen)   { min_pen = pen_left;   axis = 1; }
        if (pen_bottom < min_pen) { min_pen = pen_bottom; axis = 2; }
        if (pen_top < min_pen)    { min_pen = pen_top;    axis = 3; }

        switch (axis)
        {
            case 0: tf->position.x -= min_pen; break; /* push left  */
            case 1: tf->position.x += min_pen; break; /* push right */
            case 2: tf->position.y -= min_pen; break; /* push up    */
            case 3: tf->position.y += min_pen; break; /* push down  */
        }
    }
}

/*=============================================================================
 * move_entities
 * Query: [inout] Transform, [in] Velocity, [in] RoomOccupant, [in] Collider
 *   idx:   0                 1               2                  3
 *===========================================================================*/
void move_entities(ecs_iter_t *it)
{
    if (game_state_get()->scene != SCENE_PLAYING) return;

    Transform    *tf  = ecs_field(it, Transform, 0);
    Velocity     *vel = ecs_field(it, Velocity, 1);
    RoomOccupant *occ = ecs_field(it, RoomOccupant, 2);
    Collider     *col = ecs_field(it, Collider, 3);

    float dt = it->delta_time;

    for (int i = 0; i < it->count; ++i)
    {
        if (ecs_has(it->world, it->entities[i], Carried))
            continue;

        /* Apply velocity */
        tf[i].position.x += vel[i].value.x * dt;
        tf[i].position.y += vel[i].value.y * dt;

        /* Half-extents for boundary clamping */
        float hw = col[i].rect.width  * 0.5f;
        float hh = col[i].rect.height * 0.5f;

        /* Clamp to room outer bounds (inside the walls) */
        float min_x = WALL_THICKNESS + hw;
        float max_x = PLAYFIELD_W - WALL_THICKNESS - hw;
        float min_y = WALL_THICKNESS + hh;
        float max_y = PLAYFIELD_H - WALL_THICKNESS - hh;

        tf[i].position.x = CLAMP(tf[i].position.x, min_x, max_x);
        tf[i].position.y = CLAMP(tf[i].position.y, min_y, max_y);

        /* Resolve collisions with interior walls of current room */
        const RoomDef *room = room_get(occ[i].room_id);
        if (room && room->interior_wall_count > 0)
        {
            resolve_wall_collisions(&tf[i], &col[i], room);
        }
    }
}

#include "../components/dragon.h"
#include "../components/transform.h"
#include "../components/velocity.h"
#include "../components/room_occupant.h"
#include "../components/player.h"
#include "../components/item.h"
#include "../components/carried.h"
#include "../components/collider.h"
#include "../data/rooms.h"
#include "../defines.h"
#include "../game.h"
#include "../event_bus.h"
#include "dragon.h"
#include <math.h>
#include <stdlib.h>

/*=============================================================================
 * Dragon speed / aggro constants (indexed by DragonType)
 *===========================================================================*/
static const float DRAGON_SPEEDS[]       = { 80.0f, 120.0f, 160.0f };
static const float DRAGON_AGGRO_RANGES[] = { 200.0f, 350.0f, 500.0f };

static const float BITE_DURATION   = 0.5f;
static const float BITE_DIST       = 20.0f;
static const float GRACE_DURATION  = 0.2f;
static const float YORGLE_FLEE_KEY_DIST = 150.0f;

/*=============================================================================
 * Static query for finding the player
 *===========================================================================*/
static ecs_query_t *player_query = NULL;

static void ensure_player_query(ecs_world_t *world)
{
    if (player_query) return;
    player_query = ecs_query(world, {
        .terms = {
            { .id = ecs_id(Player) },
            { .id = ecs_id(Transform) },
            { .id = ecs_id(RoomOccupant) }
        }
    });
}

/*=============================================================================
 * Static query for finding the gold key (for Yorgle flee behavior)
 *===========================================================================*/
static ecs_query_t *item_query = NULL;

static void ensure_item_query(ecs_world_t *world)
{
    if (item_query) return;
    item_query = ecs_query(world, {
        .terms = {
            { .id = ecs_id(Item) },
            { .id = ecs_id(Transform) },
            { .id = ecs_id(RoomOccupant) }
        }
    });
}

/*=============================================================================
 * AABB wall collision helper (duplicated from player.c)
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

        float el = ex - hw;
        float er = ex + hw;
        float et = ey - hh;
        float eb = ey + hh;

        float wl = wall->x;
        float wr = wall->x + wall->w;
        float wt = wall->y;
        float wb = wall->y + wall->h;

        if (el >= wr || er <= wl || et >= wb || eb <= wt)
            continue;

        float pen_right  = er - wl;
        float pen_left   = wr - el;
        float pen_bottom = eb - wt;
        float pen_top    = wb - et;

        float min_pen = pen_right;
        int axis = 0;

        if (pen_left < min_pen)   { min_pen = pen_left;   axis = 1; }
        if (pen_bottom < min_pen) { min_pen = pen_bottom; axis = 2; }
        if (pen_top < min_pen)    { min_pen = pen_top;    axis = 3; }

        switch (axis)
        {
            case 0: tf->position.x -= min_pen; break;
            case 1: tf->position.x += min_pen; break;
            case 2: tf->position.y -= min_pen; break;
            case 3: tf->position.y += min_pen; break;
        }
    }
}

/*=============================================================================
 * Helper: distance between two points
 *===========================================================================*/
static float dist2d(float x1, float y1, float x2, float y2)
{
    float dx = x2 - x1;
    float dy = y2 - y1;
    return sqrtf(dx * dx + dy * dy);
}

/*=============================================================================
 * Helper: normalize a direction vector in-place, returns length
 *===========================================================================*/
static float normalize2d(float *x, float *y)
{
    float len = sqrtf((*x) * (*x) + (*y) * (*y));
    if (len > 0.0001f)
    {
        *x /= len;
        *y /= len;
    }
    return len;
}

/*=============================================================================
 * Helper: get exit span center for a given direction
 *===========================================================================*/
static void exit_span_center(const RoomDef *room, Direction dir,
                             float *out_x, float *out_y)
{
    float span_mid = (room->exits[dir].span_min + room->exits[dir].span_max) * 0.5f;

    switch (dir)
    {
        case DIR_NORTH:
            *out_x = span_mid;
            *out_y = WALL_THICKNESS * 0.5f;
            break;
        case DIR_SOUTH:
            *out_x = span_mid;
            *out_y = PLAYFIELD_H - WALL_THICKNESS * 0.5f;
            break;
        case DIR_WEST:
            *out_x = WALL_THICKNESS * 0.5f;
            *out_y = span_mid;
            break;
        case DIR_EAST:
            *out_x = PLAYFIELD_W - WALL_THICKNESS * 0.5f;
            *out_y = span_mid;
            break;
        default:
            *out_x = PLAYFIELD_W * 0.5f;
            *out_y = PLAYFIELD_H * 0.5f;
            break;
    }
}

/*=============================================================================
 * dragon_ai
 * Query: [inout] Dragon, [inout] Transform, [inout] Velocity, [in] RoomOccupant,
 *        [in] Collider
 *   idx:   0               1                 2                 3
 *          4
 *===========================================================================*/
void dragon_ai(ecs_iter_t *it)
{
    if (game_state_get()->scene != SCENE_PLAYING) return;

    ensure_player_query(it->world);
    ensure_item_query(it->world);

    Dragon       *dragon = ecs_field(it, Dragon, 0);
    Transform    *tf     = ecs_field(it, Transform, 1);
    Velocity     *vel    = ecs_field(it, Velocity, 2);
    RoomOccupant *occ    = ecs_field(it, RoomOccupant, 3);
    Collider     *col    = ecs_field(it, Collider, 4);

    float dt = it->delta_time;

    /* Find the player via static query */
    float player_x = 0.0f, player_y = 0.0f;
    int   player_room = -1;
    bool  player_found = false;

    {
        ecs_iter_t pit = ecs_query_iter(it->world, player_query);
        while (ecs_query_next(&pit))
        {
            Transform    *ptf  = ecs_field(&pit, Transform, 1);
            RoomOccupant *pocc = ecs_field(&pit, RoomOccupant, 2);
            if (pit.count > 0)
            {
                player_x     = ptf[0].position.x;
                player_y     = ptf[0].position.y;
                player_room  = pocc[0].room_id;
                player_found = true;
            }
        }
    }

    /* Find the gold key (for Yorgle flee behavior) */
    float gold_key_x = 0.0f, gold_key_y = 0.0f;
    int   gold_key_room = -1;
    bool  gold_key_found = false;

    {
        ecs_iter_t iit = ecs_query_iter(it->world, item_query);
        while (ecs_query_next(&iit))
        {
            Item         *items  = ecs_field(&iit, Item, 0);
            Transform    *itf    = ecs_field(&iit, Transform, 1);
            RoomOccupant *iocc   = ecs_field(&iit, RoomOccupant, 2);
            for (int k = 0; k < iit.count; ++k)
            {
                if (items[k].type == ITEM_GOLD_KEY)
                {
                    gold_key_x     = itf[k].position.x;
                    gold_key_y     = itf[k].position.y;
                    gold_key_room  = iocc[k].room_id;
                    gold_key_found = true;
                    break;
                }
            }
            if (gold_key_found) break;
        }
    }

    for (int i = 0; i < it->count; ++i)
    {
        Dragon *d = &dragon[i];

        /* Tick grace timer */
        if (d->grace_timer > 0.0f)
            d->grace_timer -= dt;

        float speed = d->speed > 0.0f ? d->speed : DRAGON_SPEEDS[d->type];
        float aggro = d->aggro_range > 0.0f ? d->aggro_range : DRAGON_AGGRO_RANGES[d->type];

        float dx = tf[i].position.x;
        float dy = tf[i].position.y;
        int   dragon_room = occ[i].room_id;

        bool same_room_player = player_found && (player_room == dragon_room);
        float dist_to_player = same_room_player
            ? dist2d(dx, dy, player_x, player_y)
            : 99999.0f;

        /* ---------------------------------------------------------------
         * State machine
         * -------------------------------------------------------------*/
        switch (d->state)
        {
        case DRAGON_IDLE:
        {
            /* Yorgle: flee from gold key if nearby in same room */
            if (d->type == DRAGON_YORGLE && gold_key_found &&
                gold_key_room == dragon_room)
            {
                float dist_key = dist2d(dx, dy, gold_key_x, gold_key_y);
                if (dist_key < YORGLE_FLEE_KEY_DIST)
                {
                    d->state = DRAGON_FLEE;
                    break;
                }
            }

            /* Check if player is in aggro range */
            if (same_room_player && dist_to_player < aggro)
            {
                d->state = DRAGON_CHASE;
                break;
            }

            /* Wander: occasionally pick a new random direction */
            d->wander_timer -= dt;
            if (d->wander_timer <= 0.0f)
            {
                /* ~5% chance per second, check every tick */
                float chance = 5.0f * dt;
                float roll = (float)(rand() % 1000) / 1000.0f;
                if (roll < chance / 100.0f || d->wander_timer <= -2.0f)
                {
                    float angle = ((float)(rand() % 360)) * 3.14159f / 180.0f;
                    d->wander_dir_x = cosf(angle);
                    d->wander_dir_y = sinf(angle);
                    d->wander_timer = 1.0f + (float)(rand() % 3);
                }
            }

            vel[i].value.x = d->wander_dir_x * speed * 0.3f;
            vel[i].value.y = d->wander_dir_y * speed * 0.3f;
            break;
        }

        case DRAGON_CHASE:
        {
            if (!player_found)
            {
                d->state = DRAGON_IDLE;
                vel[i].value.x = 0.0f;
                vel[i].value.y = 0.0f;
                break;
            }

            /* Check for bite */
            if (same_room_player && dist_to_player < BITE_DIST &&
                d->grace_timer <= 0.0f)
            {
                d->state = DRAGON_BITE;
                d->bite_timer = BITE_DURATION;
                vel[i].value.x = 0.0f;
                vel[i].value.y = 0.0f;
                break;
            }

            float target_x, target_y;

            if (same_room_player)
            {
                /* Steer directly toward player */
                target_x = player_x;
                target_y = player_y;
            }
            else
            {
                /* Use BFS to find which exit to head toward */
                int next_room = rooms_bfs_next_room(dragon_room, player_room);
                if (next_room < 0)
                {
                    /* No path; return to idle */
                    d->state = DRAGON_IDLE;
                    vel[i].value.x = 0.0f;
                    vel[i].value.y = 0.0f;
                    break;
                }

                /* Find which exit leads to next_room */
                const RoomDef *room = room_get(dragon_room);
                Direction exit_dir = DIR_NORTH;
                bool found_exit = false;
                for (int e = 0; e < DIR_COUNT; ++e)
                {
                    if (room->exits[e].destination_room == next_room)
                    {
                        exit_dir = (Direction)e;
                        found_exit = true;
                        break;
                    }
                }

                if (!found_exit)
                {
                    d->state = DRAGON_IDLE;
                    vel[i].value.x = 0.0f;
                    vel[i].value.y = 0.0f;
                    break;
                }

                exit_span_center(room, exit_dir, &target_x, &target_y);
            }

            float mx = target_x - dx;
            float my = target_y - dy;
            normalize2d(&mx, &my);

            vel[i].value.x = mx * speed;
            vel[i].value.y = my * speed;

            /* If player left aggro range and is in same room, return to idle */
            if (same_room_player && dist_to_player > aggro * 1.5f)
            {
                d->state = DRAGON_IDLE;
            }
            break;
        }

        case DRAGON_FLEE:
        {
            /* Yorgle flees from gold key */
            if (d->type == DRAGON_YORGLE && gold_key_found &&
                gold_key_room == dragon_room)
            {
                float flee_dx = dx - gold_key_x;
                float flee_dy = dy - gold_key_y;
                float dist_key = dist2d(dx, dy, gold_key_x, gold_key_y);

                if (dist_key < YORGLE_FLEE_KEY_DIST * 2.0f)
                {
                    normalize2d(&flee_dx, &flee_dy);
                    vel[i].value.x = flee_dx * speed;
                    vel[i].value.y = flee_dy * speed;
                }
                else
                {
                    /* Far enough, return to idle */
                    d->state = DRAGON_IDLE;
                    vel[i].value.x = 0.0f;
                    vel[i].value.y = 0.0f;
                }
            }
            else
            {
                /* Nothing to flee from, return to idle */
                d->state = DRAGON_IDLE;
                vel[i].value.x = 0.0f;
                vel[i].value.y = 0.0f;
            }
            break;
        }

        case DRAGON_BITE:
        {
            vel[i].value.x = 0.0f;
            vel[i].value.y = 0.0f;

            d->bite_timer -= dt;
            if (d->bite_timer <= 0.0f)
            {
                event_bus_publish(EVENT_PLAYER_DIED, NULL);
                d->state = DRAGON_IDLE;
            }
            break;
        }

        case DRAGON_DEAD:
        {
            vel[i].value.x = 0.0f;
            vel[i].value.y = 0.0f;
            break;
        }
        }

        /* Clamp to room bounds */
        float hw = col[i].rect.width  * 0.5f;
        float hh = col[i].rect.height * 0.5f;

        float min_x = WALL_THICKNESS + hw;
        float max_x = PLAYFIELD_W - WALL_THICKNESS - hw;
        float min_y = WALL_THICKNESS + hh;
        float max_y = PLAYFIELD_H - WALL_THICKNESS - hh;

        tf[i].position.x = CLAMP(tf[i].position.x, min_x, max_x);
        tf[i].position.y = CLAMP(tf[i].position.y, min_y, max_y);

        /* Resolve interior wall collisions */
        const RoomDef *room = room_get(occ[i].room_id);
        if (room && room->interior_wall_count > 0)
        {
            resolve_wall_collisions(&tf[i], &col[i], room);
        }
    }
}

#include "../components/item.h"
#include "../components/dragon.h"
#include "../components/transform.h"
#include "../components/room_occupant.h"
#include "../components/bat.h"
#include "../components/player.h"
#include "../components/carried.h"
#include "../data/rooms.h"
#include "../defines.h"
#include "../game.h"
#include "magnet.h"
#include <math.h>

//==============================================================================
// Helpers
//==============================================================================

typedef struct {
    ecs_entity_t entity;
    float        world_dist;
} _MagnetCandidate;

static float _euclidean_dist(float ax, float ay, float bx, float by)
{
    float dx = bx - ax;
    float dy = by - ay;
    return sqrtf(dx * dx + dy * dy);
}

//==============================================================================

void magnet_pull(ecs_iter_t *it)
{
    GameState *gs = game_state_get();
    if (gs->scene != SCENE_PLAYING) return;

    ecs_world_t *world = it->world;
    float dt = it->delta_time;

    /* --- Step 1: Find the magnet entity --- */
    static ecs_query_t *magnet_q = NULL;
    if (!magnet_q)
    {
        magnet_q = ecs_query(world, {
            .terms = {
                { ecs_id(Item),         .inout = EcsIn },
                { ecs_id(Transform),    .inout = EcsIn },
                { ecs_id(RoomOccupant), .inout = EcsIn },
            }
        });
    }

    ecs_entity_t magnet_ent = 0;
    vector2 magnet_pos = {0};
    int magnet_room = -1;

    {
        ecs_iter_t qit = ecs_query_iter(world, magnet_q);
        while (ecs_query_next(&qit))
        {
            Item         *items = ecs_field(&qit, Item,         0);
            Transform    *tfs   = ecs_field(&qit, Transform,    1);
            RoomOccupant *occs  = ecs_field(&qit, RoomOccupant, 2);

            for (int i = 0; i < qit.count; ++i)
            {
                if (items[i].type == ITEM_MAGNET)
                {
                    magnet_ent  = qit.entities[i];
                    magnet_pos  = tfs[i].position;
                    magnet_room = occs[i].room_id;
                    break;
                }
            }
            if (magnet_ent) break;
        }
    }

    if (!magnet_ent) return;

    /* Magnet is always active per spec, even when carried */

    /* --- Step 2: Find all movable entities (items except magnet, and dragons) --- */
    #define MAX_CANDIDATES 64
    _MagnetCandidate candidates[MAX_CANDIDATES];
    int candidate_count = 0;

    /* Scan items */
    {
        ecs_iter_t qit = ecs_query_iter(world, magnet_q);
        while (ecs_query_next(&qit))
        {
            Transform    *tfs   = ecs_field(&qit, Transform,    1);
            RoomOccupant *occs  = ecs_field(&qit, RoomOccupant, 2);

            for (int i = 0; i < qit.count; ++i)
            {
                ecs_entity_t e = qit.entities[i];
                if (e == magnet_ent) continue;
                if (ecs_has(world, e, Carried)) continue;

                float dist;
                if (occs[i].room_id == magnet_room)
                    dist = _euclidean_dist(tfs[i].position.x, tfs[i].position.y,
                                           magnet_pos.x, magnet_pos.y);
                else
                    dist = (float)room_distances[occs[i].room_id][magnet_room] * 960.0f
                         + _euclidean_dist(tfs[i].position.x, tfs[i].position.y,
                                           PLAYFIELD_W * 0.5f, PLAYFIELD_H * 0.5f);

                if (candidate_count < MAX_CANDIDATES)
                {
                    candidates[candidate_count++] = (_MagnetCandidate){
                        .entity = e, .world_dist = dist
                    };
                }
            }
        }
    }

    /* Scan dragons */
    {
        static ecs_query_t *dragon_q = NULL;
        if (!dragon_q)
        {
            dragon_q = ecs_query(world, {
                .terms = {
                    { ecs_id(Dragon),       .inout = EcsIn },
                    { ecs_id(Transform),    .inout = EcsIn },
                    { ecs_id(RoomOccupant), .inout = EcsIn },
                }
            });
        }

        ecs_iter_t qit = ecs_query_iter(world, dragon_q);
        while (ecs_query_next(&qit))
        {
            Dragon       *drags = ecs_field(&qit, Dragon,       0);
            Transform    *tfs   = ecs_field(&qit, Transform,    1);
            RoomOccupant *occs  = ecs_field(&qit, RoomOccupant, 2);

            for (int i = 0; i < qit.count; ++i)
            {
                ecs_entity_t e = qit.entities[i];
                if (ecs_has(world, e, Carried)) continue;
                if (drags[i].state == DRAGON_DEAD) continue;

                float dist;
                if (occs[i].room_id == magnet_room)
                    dist = _euclidean_dist(tfs[i].position.x, tfs[i].position.y,
                                           magnet_pos.x, magnet_pos.y);
                else
                    dist = (float)room_distances[occs[i].room_id][magnet_room] * 960.0f
                         + _euclidean_dist(tfs[i].position.x, tfs[i].position.y,
                                           PLAYFIELD_W * 0.5f, PLAYFIELD_H * 0.5f);

                if (candidate_count < MAX_CANDIDATES)
                {
                    candidates[candidate_count++] = (_MagnetCandidate){
                        .entity = e, .world_dist = dist
                    };
                }
            }
        }
    }

    /* --- Step 3: Find the 2 nearest --- */
    int nearest[2] = { -1, -1 };
    float nearest_dist[2] = { 1e18f, 1e18f };

    for (int c = 0; c < candidate_count; ++c)
    {
        if (candidates[c].world_dist < nearest_dist[0])
        {
            nearest[1]      = nearest[0];
            nearest_dist[1] = nearest_dist[0];
            nearest[0]      = c;
            nearest_dist[0] = candidates[c].world_dist;
        }
        else if (candidates[c].world_dist < nearest_dist[1])
        {
            nearest[1]      = c;
            nearest_dist[1] = candidates[c].world_dist;
        }
    }

    /* --- Step 4: Pull each toward the magnet --- */
    float pull_speed = 30.0f;

    for (int n = 0; n < 2; ++n)
    {
        if (nearest[n] < 0) continue;

        ecs_entity_t e = candidates[nearest[n]].entity;
        if (!ecs_is_alive(world, e)) continue;

        Transform    *etf  = ecs_get_mut(world, e, Transform);
        RoomOccupant *eocc = ecs_get_mut(world, e, RoomOccupant);
        if (!etf || !eocc) continue;

        if (eocc->room_id == magnet_room)
        {
            /* Same room: move directly toward magnet */
            float dx = magnet_pos.x - etf->position.x;
            float dy = magnet_pos.y - etf->position.y;
            float dist = sqrtf(dx * dx + dy * dy);

            if (dist > 1.0f)
            {
                float step = pull_speed * dt;
                if (step > dist) step = dist;
                etf->position.x += (dx / dist) * step;
                etf->position.y += (dy / dist) * step;
            }
        }
        else
        {
            /* Different room: move toward exit leading closer */
            int next_room = rooms_bfs_next_room(eocc->room_id, magnet_room);
            if (next_room == ROOM_NONE) continue;

            const RoomDef *cur_room = room_get(eocc->room_id);

            /* Find which exit leads to next_room */
            for (int d = 0; d < DIR_COUNT; ++d)
            {
                if (cur_room->exits[d].destination_room != next_room) continue;

                /* Target the center of that exit span */
                float tx, ty;
                float span_mid = (cur_room->exits[d].span_min + cur_room->exits[d].span_max) * 0.5f;

                switch (d)
                {
                    case DIR_NORTH:
                        tx = span_mid;
                        ty = WALL_THICKNESS * 0.5f;
                        break;
                    case DIR_SOUTH:
                        tx = span_mid;
                        ty = PLAYFIELD_H - WALL_THICKNESS * 0.5f;
                        break;
                    case DIR_EAST:
                        tx = PLAYFIELD_W - WALL_THICKNESS * 0.5f;
                        ty = span_mid;
                        break;
                    case DIR_WEST:
                        tx = WALL_THICKNESS * 0.5f;
                        ty = span_mid;
                        break;
                    default:
                        tx = etf->position.x;
                        ty = etf->position.y;
                        break;
                }

                float dx = tx - etf->position.x;
                float dy = ty - etf->position.y;
                float dist = sqrtf(dx * dx + dy * dy);

                if (dist > 1.0f)
                {
                    float step = pull_speed * dt;
                    if (step > dist) step = dist;
                    etf->position.x += (dx / dist) * step;
                    etf->position.y += (dy / dist) * step;
                }
                break;
            }
        }
    }

    #undef MAX_CANDIDATES
}

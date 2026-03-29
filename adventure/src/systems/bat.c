#include "../components/bat.h"
#include "../components/transform.h"
#include "../components/velocity.h"
#include "../components/room_occupant.h"
#include "../components/carryable.h"
#include "../components/carried.h"
#include "../components/player.h"
#include "../components/dragon.h"
#include "../components/item.h"
#include "../components/collider.h"
#include "../data/rooms.h"
#include "../defines.h"
#include "../game.h"
#include "../event_bus.h"
#include "../managers/audio.h"
#include "bat.h"
#include <math.h>
#include <stdlib.h>

//==============================================================================
// Helpers
//==============================================================================

static float _rand_range(float lo, float hi)
{
    return lo + ((float)rand() / (float)RAND_MAX) * (hi - lo);
}

static float _rand_heading(void)
{
    float angle = _rand_range(0.0f, 2.0f * 3.14159265f);
    return angle;
}

static bool _overlaps(float ax, float ay, float aw, float ah,
                      float bx, float by, float bw, float bh)
{
    float ahw = aw * 0.5f, ahh = ah * 0.5f;
    float bhw = bw * 0.5f, bhh = bh * 0.5f;
    return fabsf(ax - bx) < (ahw + bhw) && fabsf(ay - by) < (ahh + bhh);
}

//==============================================================================

void bat_ai(ecs_iter_t *it)
{
    GameState *gs = game_state_get();
    if (gs->scene != SCENE_PLAYING) return;

    Bat          *bat   = ecs_field(it, Bat,          0);
    Transform    *tf    = ecs_field(it, Transform,    1);
    Velocity     *vel   = ecs_field(it, Velocity,     2);
    RoomOccupant *occ   = ecs_field(it, RoomOccupant, 3);
    Collider     *col   = ecs_field(it, Collider,     4);

    float dt = it->delta_time;

    for (int i = 0; i < it->count; ++i)
    {
        ecs_entity_t bat_ent = it->entities[i];

        /* --- Update cooldowns --- */
        bat[i].swap_cooldown -= dt;
        if (bat[i].swap_cooldown < 0.0f) bat[i].swap_cooldown = 0.0f;

        bat[i].retarget_timer -= dt;

        /* --- Choose new heading periodically --- */
        if (bat[i].retarget_timer <= 0.0f)
        {
            float angle = _rand_heading();
            bat[i].heading_x = cosf(angle);
            bat[i].heading_y = sinf(angle);
            bat[i].retarget_timer = _rand_range(1.0f, 3.0f);
        }

        const RoomDef *room = room_get(occ[i].room_id);
        float spd = bat[i].speed > 0.0f ? bat[i].speed : 150.0f;
        float next_x = tf[i].position.x + bat[i].heading_x * spd * dt;
        float next_y = tf[i].position.y + bat[i].heading_y * spd * dt;
        float hw = col[i].rect.width * 0.5f;
        float hh = col[i].rect.height * 0.5f;

        float min_x = WALL_THICKNESS + hw;
        float max_x = PLAYFIELD_W - WALL_THICKNESS - hw;
        float min_y = WALL_THICKNESS + hh;
        float max_y = PLAYFIELD_H - WALL_THICKNESS - hh;

        if (next_x < min_x || next_x > max_x)
            bat[i].heading_x = -bat[i].heading_x;
        if (next_y < min_y || next_y > max_y)
            bat[i].heading_y = -bat[i].heading_y;

        /* --- Push out of interior walls --- */
        if (room)
        {
            next_x = tf[i].position.x + bat[i].heading_x * spd * dt;
            next_y = tf[i].position.y + bat[i].heading_y * spd * dt;

            for (int w = 0; w < room->interior_wall_count; ++w)
            {
                const WallRect *wall = &room->interior_walls[w];
                float wx = wall->x + wall->w * 0.5f;
                float wy = wall->y + wall->h * 0.5f;

                if (_overlaps(next_x, next_y, col[i].rect.width, col[i].rect.height,
                              wx, wy, wall->w, wall->h))
                {
                    float dx = next_x - wx;
                    float dy = next_y - wy;
                    float overlap_x = (hw + wall->w * 0.5f) - fabsf(dx);
                    float overlap_y = (hh + wall->h * 0.5f) - fabsf(dy);

                    if (overlap_x < overlap_y)
                        bat[i].heading_x = -bat[i].heading_x;
                    else
                        bat[i].heading_y = -bat[i].heading_y;
                }
            }
        }

        vel[i].value.x = bat[i].heading_x * spd;
        vel[i].value.y = bat[i].heading_y * spd;

        /* --- Try to pick up / swap carryable entities --- */
        if (bat[i].swap_cooldown <= 0.0f)
        {
            /* Build a query for carryable entities */
            static ecs_query_t *carry_q = NULL;
            if (!carry_q)
            {
                carry_q = ecs_query(it->world, {
                    .terms = {
                        { ecs_id(Transform),    .inout = EcsIn },
                        { ecs_id(Carryable),    .inout = EcsIn },
                        { ecs_id(RoomOccupant), .inout = EcsIn },
                    }
                });
            }

            ecs_entity_t best_target = 0;
            float best_dist = 1e9f;

            ecs_iter_t qit = ecs_query_iter(it->world, carry_q);
            while (ecs_query_next(&qit))
            {
                Transform    *ct = ecs_field(&qit, Transform,    0);
                Carryable    *cc = ecs_field(&qit, Carryable,    1);
                RoomOccupant *co = ecs_field(&qit, RoomOccupant, 2);

                for (int j = 0; j < qit.count; ++j)
                {
                    ecs_entity_t target = qit.entities[j];

                    /* Skip self */
                    if (target == bat_ent) continue;

                    /* Must be bat-carryable */
                    if (!cc[j].bat_can_carry) continue;

                    /* Never carry the player */
                    if (ecs_has(it->world, target, Player)) continue;

                    /* Must be in same room */
                    if (co[j].room_id != occ[i].room_id) continue;

                    /* Check if currently carried by someone else (not bat) */
                    const Carried *carried = ecs_get(it->world, target, Carried);
                    if (carried && carried->carrier == bat_ent) continue; /* already carrying this */

                    /* Check overlap */
                    if (!_overlaps(tf[i].position.x, tf[i].position.y, 24.0f, 24.0f,
                                   ct[j].position.x, ct[j].position.y, 16.0f, 16.0f))
                        continue;

                    float dx = ct[j].position.x - tf[i].position.x;
                    float dy = ct[j].position.y - tf[i].position.y;
                    float dist = dx * dx + dy * dy;

                    if (dist < best_dist)
                    {
                        best_dist = dist;
                        best_target = target;
                    }
                }
            }

            /* Also consider stealing from the player */
            if (!best_target)
            {
                static ecs_query_t *player_q = NULL;
                if (!player_q)
                {
                    player_q = ecs_query(it->world, {
                        .terms = {
                            { ecs_id(Player),       .inout = EcsIn },
                            { ecs_id(Transform),    .inout = EcsIn },
                            { ecs_id(RoomOccupant), .inout = EcsIn },
                        }
                    });
                }

                ecs_iter_t pit = ecs_query_iter(it->world, player_q);
                while (ecs_query_next(&pit))
                {
                    Player       *pp = ecs_field(&pit, Player,       0);
                    Transform    *pt = ecs_field(&pit, Transform,    1);
                    RoomOccupant *po = ecs_field(&pit, RoomOccupant, 2);

                    for (int j = 0; j < pit.count; ++j)
                    {
                        if (po[j].room_id != occ[i].room_id) continue;
                        if (!pp[j].carried_entity) continue;

                        /* Check overlap with player */
                        if (_overlaps(tf[i].position.x, tf[i].position.y, 24.0f, 24.0f,
                                      pt[j].position.x, pt[j].position.y, 16.0f, 16.0f))
                        {
                            /* Steal the player's item */
                            ecs_entity_t stolen = pp[j].carried_entity;

                            /* Verify the item is bat-carryable */
                            const Carryable *sc = ecs_get(it->world, stolen, Carryable);
                            if (sc && sc->bat_can_carry)
                            {
                                best_target = stolen;

                                /* Remove from player */
                                Player *pp_mut = ecs_get_mut(it->world, pit.entities[j], Player);
                                pp_mut->carried_entity = 0;
                                ecs_remove(it->world, stolen, Carried);
                            }
                        }
                    }
                }
            }

            if (best_target)
            {
                /* Drop currently carried entity, if any */
                if (bat[i].carried_entity)
                {
                    ecs_entity_t old = bat[i].carried_entity;
                    if (ecs_is_alive(it->world, old))
                    {
                        ecs_remove(it->world, old, Carried);

                        /* Place dropped entity at bat position */
                        Transform *old_tf = ecs_get_mut(it->world, old, Transform);
                        if (old_tf)
                        {
                            old_tf->position.x = tf[i].position.x;
                            old_tf->position.y = tf[i].position.y;
                        }

                        /* Ensure it's in the bat's room */
                        RoomOccupant *old_occ = ecs_get_mut(it->world, old, RoomOccupant);
                        if (old_occ)
                            old_occ->room_id = occ[i].room_id;
                    }
                    bat[i].carried_entity = 0;
                }

                /* Pick up new target */
                bat[i].carried_entity = best_target;
                ecs_set(it->world, best_target, Carried, {
                    .carrier = bat_ent,
                    .local_offset = { 0.0f, 12.0f }
                });

                bat[i].swap_cooldown = 1.0f;

                event_bus_publish(EVENT_PLAY_SOUND,
                    &(PlaySoundData){ .id = SOUND_BAT_SWAP, .volume = 0.5f });
            }
        }

        if (bat[i].carried_entity && !ecs_is_alive(it->world, bat[i].carried_entity))
            bat[i].carried_entity = 0;
    }
}

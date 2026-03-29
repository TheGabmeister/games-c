#include "../components/player.h"
#include "../components/dragon.h"
#include "../components/item.h"
#include "../components/transform.h"
#include "../components/room_occupant.h"
#include "../components/carryable.h"
#include "../components/carried.h"
#include "../components/gate.h"
#include "../components/velocity.h"
#include "../components/collider.h"
#include "../components/input.h"
#include "../data/rooms.h"
#include "../defines.h"
#include "../game.h"
#include "../event_bus.h"
#include "../managers/audio.h"
#include "interaction.h"
#include <math.h>

//==============================================================================
// Helpers
//==============================================================================

static bool _overlaps(float ax, float ay, float aw, float ah,
                      float bx, float by, float bw, float bh)
{
    float ahw = aw * 0.5f, ahh = ah * 0.5f;
    float bhw = bw * 0.5f, bhh = bh * 0.5f;
    return fabsf(ax - bx) < (ahw + bhw) && fabsf(ay - by) < (ahh + bhh);
}

static ItemType _key_type_to_item_type(KeyType kt)
{
    switch (kt)
    {
        case KEY_GOLD:  return ITEM_GOLD_KEY;
        case KEY_BLACK: return ITEM_BLACK_KEY;
        case KEY_WHITE: return ITEM_WHITE_KEY;
        default:        return ITEM_TYPE_COUNT;
    }
}

//==============================================================================
// resolve_player_interactions
//==============================================================================

void resolve_player_interactions(ecs_iter_t *it)
{
    GameState *gs = game_state_get();
    if (gs->scene != SCENE_PLAYING) return;

    ecs_world_t *world = it->world;

    /* --- Find the player --- */
    static ecs_query_t *player_q = NULL;
    if (!player_q)
    {
        player_q = ecs_query(world, {
            .terms = {
                { ecs_id(Player),       .inout = EcsInOut },
                { ecs_id(Transform),    .inout = EcsIn },
                { ecs_id(RoomOccupant), .inout = EcsIn },
            }
        });
    }

    ecs_entity_t player_ent = 0;
    Player      *player     = NULL;
    vector2      player_pos = {0};
    int          player_room = -1;

    {
        ecs_iter_t qit = ecs_query_iter(world, player_q);
        while (ecs_query_next(&qit))
        {
            Player       *pp = ecs_field(&qit, Player,       0);
            Transform    *pt = ecs_field(&qit, Transform,    1);
            RoomOccupant *po = ecs_field(&qit, RoomOccupant, 2);

            for (int i = 0; i < qit.count; ++i)
            {
                player_ent  = qit.entities[i];
                player      = &pp[i];
                player_pos  = pt[i].position;
                player_room = po[i].room_id;
                break;
            }
            if (player_ent) break;
        }
    }

    if (!player_ent) return;

    /* --- Priority 1: Sword kills dragon --- */
    bool player_has_sword = false;
    if (player->carried_entity && ecs_is_alive(world, player->carried_entity))
    {
        const Item *ci = ecs_get(world, player->carried_entity, Item);
        if (ci && ci->type == ITEM_SWORD)
            player_has_sword = true;
    }

    /* --- Find dragons --- */
    static ecs_query_t *dragon_q = NULL;
    if (!dragon_q)
    {
        dragon_q = ecs_query(world, {
            .terms = {
                { ecs_id(Dragon),       .inout = EcsInOut },
                { ecs_id(Transform),    .inout = EcsIn },
                { ecs_id(RoomOccupant), .inout = EcsIn },
            }
        });
    }

    {
        ecs_iter_t qit = ecs_query_iter(world, dragon_q);
        while (ecs_query_next(&qit))
        {
            Dragon       *drags = ecs_field(&qit, Dragon,       0);
            Transform    *dtfs  = ecs_field(&qit, Transform,    1);
            RoomOccupant *doccs = ecs_field(&qit, RoomOccupant, 2);

            for (int i = 0; i < qit.count; ++i)
            {
                if (drags[i].state == DRAGON_DEAD) continue;
                if (doccs[i].room_id != player_room) continue;

                bool overlap = _overlaps(
                    player_pos.x, player_pos.y, PLAYER_SIZE, PLAYER_SIZE,
                    dtfs[i].position.x, dtfs[i].position.y, 24.0f, 24.0f);

                if (!overlap) continue;

                /* Sword kills dragon (priority 1) */
                if (player_has_sword)
                {
                    drags[i].state = DRAGON_DEAD;
                    event_bus_publish(EVENT_DRAGON_KILLED,
                        &(DragonKilledData){ .dragon_type = drags[i].type });
                    event_bus_publish(EVENT_PLAY_SOUND,
                        &(PlaySoundData){ .id = SOUND_DRAGON_DEATH, .volume = 1.0f });
                    continue;
                }

                /* Dragon bites player (priority 2) */
                if (drags[i].grace_timer <= 0.0f && drags[i].state != DRAGON_BITE)
                {
                    drags[i].state = DRAGON_BITE;
                    event_bus_publish(EVENT_PLAYER_DIED, NULL);

                    /* Set scene to death freeze */
                    gs->scene = SCENE_DEATH_FREEZE;
                    gs->death_timer = 1.5f;

                    /* Drop player's carried item */
                    if (player->carried_entity && ecs_is_alive(world, player->carried_entity))
                    {
                        ecs_remove(world, player->carried_entity, Carried);
                        player->carried_entity = 0;
                    }

                    return; /* Stop processing after death */
                }
            }
        }
    }

    /* --- Priority 4: Player interact (pickup / swap / drop) --- */
    const Input *input = ecs_singleton_get(world, Input);
    if (!input || !input->interact_pressed) return;

    /* Find overlapping carryable entities */
    static ecs_query_t *carry_q = NULL;
    if (!carry_q)
    {
        carry_q = ecs_query(world, {
            .terms = {
                { ecs_id(Transform),    .inout = EcsIn },
                { ecs_id(Carryable),    .inout = EcsIn },
                { ecs_id(RoomOccupant), .inout = EcsIn },
            }
        });
    }

    ecs_entity_t best_pickup = 0;
    float best_dist = 1e9f;

    {
        ecs_iter_t qit = ecs_query_iter(world, carry_q);
        while (ecs_query_next(&qit))
        {
            Transform    *ct = ecs_field(&qit, Transform,    0);
            Carryable    *cc = ecs_field(&qit, Carryable,    1);
            RoomOccupant *co = ecs_field(&qit, RoomOccupant, 2);

            for (int j = 0; j < qit.count; ++j)
            {
                ecs_entity_t e = qit.entities[j];
                if (!cc[j].player_can_carry) continue;
                if (co[j].room_id != player_room) continue;

                /* Skip item player is already carrying */
                if (player->carried_entity == e) continue;

                /* Skip items carried by someone else */
                const Carried *cr = ecs_get(world, e, Carried);
                if (cr && cr->carrier != player_ent) continue;

                if (!_overlaps(player_pos.x, player_pos.y, PLAYER_SIZE, PLAYER_SIZE,
                               ct[j].position.x, ct[j].position.y, 16.0f, 16.0f))
                    continue;

                float dx = ct[j].position.x - player_pos.x;
                float dy = ct[j].position.y - player_pos.y;
                float dist = dx * dx + dy * dy;

                if (dist < best_dist)
                {
                    best_dist = dist;
                    best_pickup = e;
                }
            }
        }
    }

    if (player->carried_entity && best_pickup)
    {
        /* Swap: drop current, pick up new */
        ecs_entity_t old = player->carried_entity;
        if (ecs_is_alive(world, old))
        {
            ecs_remove(world, old, Carried);
            Transform *old_tf = ecs_get_mut(world, old, Transform);
            if (old_tf)
            {
                old_tf->position.x = player_pos.x;
                old_tf->position.y = player_pos.y;
            }
        }

        player->carried_entity = best_pickup;
        ecs_set(world, best_pickup, Carried, {
            .carrier = player_ent,
            .local_offset = { 0.0f, 20.0f }
        });

        event_bus_publish(EVENT_PLAY_SOUND,
            &(PlaySoundData){ .id = SOUND_PICKUP, .volume = 0.7f });
    }
    else if (!player->carried_entity && best_pickup)
    {
        /* Pick up */
        player->carried_entity = best_pickup;
        ecs_set(world, best_pickup, Carried, {
            .carrier = player_ent,
            .local_offset = { 0.0f, 20.0f }
        });

        event_bus_publish(EVENT_PLAY_SOUND,
            &(PlaySoundData){ .id = SOUND_PICKUP, .volume = 0.7f });
    }
    else if (player->carried_entity && !best_pickup)
    {
        /* Drop current item in front of player */
        ecs_entity_t old = player->carried_entity;
        if (ecs_is_alive(world, old))
        {
            ecs_remove(world, old, Carried);

            Transform *old_tf = ecs_get_mut(world, old, Transform);
            if (old_tf)
            {
                old_tf->position.x = player_pos.x;
                old_tf->position.y = player_pos.y + 24.0f;

                /* Clamp to room bounds */
                old_tf->position.x = CLAMP(old_tf->position.x, WALL_THICKNESS, PLAYFIELD_W - WALL_THICKNESS);
                old_tf->position.y = CLAMP(old_tf->position.y, WALL_THICKNESS, PLAYFIELD_H - WALL_THICKNESS);
            }
        }

        player->carried_entity = 0;

        event_bus_publish(EVENT_PLAY_SOUND,
            &(PlaySoundData){ .id = SOUND_DROP, .volume = 0.5f });
    }
}

//==============================================================================
// resolve_gate_state
//==============================================================================

void resolve_gate_state(ecs_iter_t *it)
{
    GameState *gs = game_state_get();
    if (gs->scene != SCENE_PLAYING) return;

    ecs_world_t *world = it->world;

    /* Query all gates */
    static ecs_query_t *gate_q = NULL;
    if (!gate_q)
    {
        gate_q = ecs_query(world, {
            .terms = {
                { ecs_id(Gate), .inout = EcsInOut },
            }
        });
    }

    /* Query all items with room info */
    static ecs_query_t *item_q = NULL;
    if (!item_q)
    {
        item_q = ecs_query(world, {
            .terms = {
                { ecs_id(Item),         .inout = EcsIn },
                { ecs_id(RoomOccupant), .inout = EcsIn },
            }
        });
    }

    ecs_iter_t git = ecs_query_iter(world, gate_q);
    while (ecs_query_next(&git))
    {
        Gate *gates = ecs_field(&git, Gate, 0);

        for (int i = 0; i < git.count; ++i)
        {
            ItemType needed = _key_type_to_item_type(gates[i].key_type);
            bool key_in_room = false;

            /* Search for matching key in same room */
            ecs_iter_t iit = ecs_query_iter(world, item_q);
            while (ecs_query_next(&iit))
            {
                Item         *items = ecs_field(&iit, Item,         0);
                RoomOccupant *occs  = ecs_field(&iit, RoomOccupant, 1);

                for (int j = 0; j < iit.count; ++j)
                {
                    if (items[j].type == needed && occs[j].room_id == gates[i].room_id)
                    {
                        key_in_room = true;
                        break;
                    }
                }
                if (key_in_room) break;
            }

            bool should_be_open = key_in_room;
            if (gates[i].open != should_be_open)
            {
                gates[i].open = should_be_open;

                event_bus_publish(EVENT_GATE_STATE_CHANGED,
                    &(GateStateChangedData){
                        .room_id   = gates[i].room_id,
                        .direction = gates[i].direction,
                        .open      = should_be_open
                    });

                event_bus_publish(EVENT_PLAY_SOUND,
                    &(PlaySoundData){ .id = should_be_open ? SOUND_GATE_OPEN : SOUND_GATE_CLOSE, .volume = 0.8f });
            }
        }
    }
}

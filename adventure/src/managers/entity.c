#include "../platform.h"
#include <SDL3/SDL.h>

#include "../components/transform.h"
#include "../components/velocity.h"
#include "../components/collider.h"
#include "../components/room_occupant.h"
#include "../components/player.h"
#include "../components/dragon.h"
#include "../components/bat.h"
#include "../components/item.h"
#include "../components/carryable.h"
#include "../components/gate.h"
#include "../components/shape.h"
#include "../data/rooms.h"
#include "../data/game_modes.h"
#include "../defines.h"

#include "entity.h"

/*=============================================================================
 * Tags
 *===========================================================================*/
void entity_manager_init(ecs_world_t *world)
{
    (void)world;
}

/*=============================================================================
 * Spawn helpers
 *===========================================================================*/
static ecs_entity_t _spawn_player(ecs_world_t *w, int room, float x, float y)
{
    ecs_entity_t e = ecs_new(w);
    ecs_set(w, e, Player,       { .carried_entity = 0 });
    ecs_set(w, e, Transform,    { .position = {x, y} });
    ecs_set(w, e, Velocity,     { .value = {0, 0} });
    ecs_set(w, e, Collider,     { .layer = COL_LAYER_PLAYER,
                                  .mask  = COL_LAYER_WALL | COL_LAYER_DRAGON | COL_LAYER_ITEM | COL_LAYER_BAT,
                                  .type  = COLLIDER_RECT,
                                  .rect  = { PLAYER_SIZE, PLAYER_SIZE } });
    ecs_set(w, e, RoomOccupant, { .room_id = room });
    ecs_set(w, e, Shape,        { .type = SHAPE_RECTANGLE,
                                  .color = {255, 215, 0, 255},
                                  .rectangle = { PLAYER_SIZE, PLAYER_SIZE } });
    return e;
}

static ecs_entity_t _spawn_item(ecs_world_t *w, ItemType type, int room, float x, float y)
{
    ecs_entity_t e = ecs_new(w);
    ecs_set(w, e, Item,         { .type = type });
    ecs_set(w, e, Transform,    { .position = {x, y} });
    ecs_set(w, e, Velocity,     { .value = {0, 0} });
    ecs_set(w, e, RoomOccupant, { .room_id = room });

    /* Carryable permissions */
    bool player_carry = true;
    bool bat_carry    = true;

    /* Item-specific collider sizes */
    float cw = 10.0f, ch = 18.0f;
    switch (type)
    {
        case ITEM_SWORD:      cw = 6;  ch = 28; break;
        case ITEM_GOLD_KEY:   cw = 10; ch = 18; break;
        case ITEM_BLACK_KEY:  cw = 10; ch = 18; break;
        case ITEM_WHITE_KEY:  cw = 10; ch = 18; break;
        case ITEM_MAGNET:     cw = 16; ch = 16; break;
        case ITEM_BRIDGE:     cw = 40; ch = 8;  break;
        case ITEM_CHALICE:    cw = 12; ch = 20; break;
        case ITEM_SECRET_DOT: cw = 6;  ch = 6; player_carry = true; bat_carry = false; break;
        default: break;
    }

    ecs_set(w, e, Carryable, { .player_can_carry = player_carry, .bat_can_carry = bat_carry });
    ecs_set(w, e, Collider,  { .layer = COL_LAYER_ITEM,
                               .mask  = COL_LAYER_WALL | COL_LAYER_PLAYER | COL_LAYER_BAT,
                               .type  = COLLIDER_RECT,
                               .rect  = { cw, ch } });
    return e;
}

static ecs_entity_t _spawn_dragon(ecs_world_t *w, DragonType type, int room, float x, float y)
{
    float speed, aggro, flee_range;
    float dw, dh;

    switch (type)
    {
        case DRAGON_YORGLE:
            speed = 80; aggro = 200; flee_range = 150;
            dw = 28; dh = 20;
            break;
        case DRAGON_GRUNDLE:
            speed = 120; aggro = 350; flee_range = 0;
            dw = 24; dh = 18;
            break;
        case DRAGON_RHINDLE:
            speed = 160; aggro = 500; flee_range = 0;
            dw = 24; dh = 18;
            break;
        default:
            speed = 100; aggro = 300; flee_range = 0;
            dw = 24; dh = 18;
            break;
    }

    ecs_entity_t e = ecs_new(w);
    ecs_set(w, e, Dragon,       { .type = type, .state = DRAGON_IDLE,
                                  .home_room = room, .speed = speed,
                                  .aggro_range = aggro, .flee_range = flee_range,
                                  .bite_timer = 0, .grace_timer = 0,
                                  .wander_timer = 0, .wander_dir_x = 0, .wander_dir_y = 0 });
    ecs_set(w, e, Transform,    { .position = {x, y} });
    ecs_set(w, e, Velocity,     { .value = {0, 0} });
    ecs_set(w, e, Collider,     { .layer = COL_LAYER_DRAGON,
                                  .mask  = COL_LAYER_WALL | COL_LAYER_PLAYER | COL_LAYER_BAT,
                                  .type  = COLLIDER_RECT,
                                  .rect  = { dw, dh } });
    ecs_set(w, e, RoomOccupant, { .room_id = room });
    ecs_set(w, e, Carryable,    { .player_can_carry = false, .bat_can_carry = true });
    return e;
}

static ecs_entity_t _spawn_bat(ecs_world_t *w, int room, float x, float y)
{
    ecs_entity_t e = ecs_new(w);
    ecs_set(w, e, Bat,          { .carried_entity = 0, .heading_x = 1, .heading_y = 0,
                                  .retarget_timer = 0, .swap_cooldown = 0, .speed = 150 });
    ecs_set(w, e, Transform,    { .position = {x, y} });
    ecs_set(w, e, Velocity,     { .value = {0, 0} });
    ecs_set(w, e, Collider,     { .layer = COL_LAYER_BAT,
                                  .mask  = COL_LAYER_WALL | COL_LAYER_PLAYER | COL_LAYER_DRAGON | COL_LAYER_ITEM,
                                  .type  = COLLIDER_RECT,
                                  .rect  = { 12, 10 } });
    ecs_set(w, e, RoomOccupant, { .room_id = room });
    return e;
}

static void _spawn_gate(ecs_world_t *w, int room_id, int direction, KeyType key_type)
{
    ecs_entity_t e = ecs_new(w);
    ecs_set(w, e, Gate, { .room_id = room_id, .direction = direction,
                          .key_type = key_type, .open = false });
}

/*=============================================================================
 * entity_manager_spawn_world — populate the ECS world for a game mode
 *===========================================================================*/
void entity_manager_spawn_world(ecs_world_t *w, int game_mode)
{
    const GameModeData *mode = game_mode_get(game_mode);
    if (!mode)
    {
        SDL_Log("entity_manager: invalid game mode %d", game_mode);
        return;
    }

    /* Spawn player */
    _spawn_player(w, mode->player_room, mode->player_x, mode->player_y);

    /* Get spawns (handles randomization for mode 3) */
    EntitySpawn items[16] = {0};
    EntitySpawn dragons[4] = {0};
    int item_count = 0, dragon_count = 0;
    game_mode_get_spawns(game_mode, items, &item_count, dragons, &dragon_count);

    /* Spawn items */
    for (int i = 0; i < item_count; i++)
        _spawn_item(w, (ItemType)items[i].type, items[i].room_id, items[i].x, items[i].y);

    /* Spawn dragons */
    for (int i = 0; i < dragon_count; i++)
        _spawn_dragon(w, (DragonType)dragons[i].type, dragons[i].room_id, dragons[i].x, dragons[i].y);

    /* Spawn bat if mode has one */
    if (mode->bat_room >= 0)
        _spawn_bat(w, mode->bat_room, mode->bat_x, mode->bat_y);

    /* Spawn gates for castle entrances */
    _spawn_gate(w, ROOM_GOLD_ENTRANCE, DIR_SOUTH, KEY_GOLD);
    _spawn_gate(w, ROOM_BLACK_GATE, DIR_NORTH, KEY_BLACK);
    _spawn_gate(w, ROOM_WHITE_GATE, DIR_NORTH, KEY_WHITE);
}

/*=============================================================================
 * entity_manager_clear_gameplay — remove all gameplay entities
 *===========================================================================*/
void entity_manager_clear_gameplay(ecs_world_t *w)
{
    ecs_query_t *transform_q = ecs_query(w, {
        .terms = {{ ecs_id(Transform) }}
    });
    ecs_query_t *gate_q = ecs_query(w, {
        .terms = {{ ecs_id(Gate) }}
    });

    ecs_defer_begin(w);

    ecs_iter_t qit = ecs_query_iter(w, transform_q);
    while (ecs_query_next(&qit))
    {
        for (int i = 0; i < qit.count; ++i)
            ecs_delete(w, qit.entities[i]);
    }

    qit = ecs_query_iter(w, gate_q);
    while (ecs_query_next(&qit))
    {
        for (int i = 0; i < qit.count; ++i)
            ecs_delete(w, qit.entities[i]);
    }

    ecs_defer_end(w);

    ecs_query_fini(transform_q);
    ecs_query_fini(gate_q);
}

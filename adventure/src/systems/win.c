#include "../components/player.h"
#include "../components/item.h"
#include "../components/room_occupant.h"
#include "../data/rooms.h"
#include "../defines.h"
#include "../game.h"
#include "../event_bus.h"
#include "win.h"

//==============================================================================

void check_victory(ecs_iter_t *it)
{
    GameState *gs = game_state_get();
    if (gs->scene != SCENE_PLAYING) return;

    ecs_world_t *world = it->world;

    /* Find the player */
    static ecs_query_t *player_q = NULL;
    if (!player_q)
    {
        player_q = ecs_query(world, {
            .terms = {
                { ecs_id(Player),       .inout = EcsIn },
                { ecs_id(RoomOccupant), .inout = EcsIn },
            }
        });
    }

    ecs_iter_t qit = ecs_query_iter(world, player_q);
    while (ecs_query_next(&qit))
    {
        Player       *pp   = ecs_field(&qit, Player,       0);
        RoomOccupant *occs = ecs_field(&qit, RoomOccupant, 1);

        for (int i = 0; i < qit.count; ++i)
        {
            /* Must be in the gold throne room */
            if (occs[i].room_id != ROOM_GOLD_THRONE) continue;

            /* Must be carrying an entity */
            if (!pp[i].carried_entity) continue;
            if (!ecs_is_alive(world, pp[i].carried_entity)) continue;

            /* Check if carried entity is the chalice */
            const Item *item = ecs_get(world, pp[i].carried_entity, Item);
            if (!item) continue;
            if (item->type != ITEM_CHALICE) continue;

            /* Victory! */
            gs->scene = SCENE_VICTORY;
            event_bus_publish(EVENT_GAME_WON, NULL);
            return;
        }
    }
}

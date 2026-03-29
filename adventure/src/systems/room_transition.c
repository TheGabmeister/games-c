#include "../components/transform.h"
#include "../components/room_occupant.h"
#include "../components/collider.h"
#include "../components/carried.h"
#include "../components/dragon.h"
#include "../components/player.h"
#include "../components/gate.h"
#include "../data/rooms.h"
#include "../defines.h"
#include "../game.h"
#include "room_transition.h"

//==============================================================================

static bool _is_exit_blocked_by_gate(ecs_world_t *world, int room_id, int direction)
{
    static ecs_query_t *gate_q = NULL;
    if (!gate_q)
    {
        gate_q = ecs_query(world, {
            .terms = {
                { ecs_id(Gate), .inout = EcsIn },
            }
        });
    }

    ecs_iter_t qit = ecs_query_iter(world, gate_q);
    while (ecs_query_next(&qit))
    {
        Gate *gates = ecs_field(&qit, Gate, 0);
        for (int i = 0; i < qit.count; ++i)
        {
            if (gates[i].room_id == room_id &&
                gates[i].direction == direction &&
                !gates[i].open)
            {
                return true;
            }
        }
    }

    return false;
}

//==============================================================================

void room_transition(ecs_iter_t *it)
{
    GameState *gs = game_state_get();
    if (gs->scene != SCENE_PLAYING) return;

    Transform    *tf  = ecs_field(it, Transform,    0);
    RoomOccupant *occ = ecs_field(it, RoomOccupant, 1);
    Collider     *col = ecs_field(it, Collider,     2);

    for (int i = 0; i < it->count; ++i)
    {
        if (ecs_has(it->world, it->entities[i], Carried))
            continue;

        const RoomDef *room = room_get(occ[i].room_id);
        if (!room) continue;

        float px = tf[i].position.x;
        float py = tf[i].position.y;
        float hw = (col[i].type == COLLIDER_RECT) ? (col[i].rect.width * 0.5f) : col[i].circle.radius;
        float hh = (col[i].type == COLLIDER_RECT) ? (col[i].rect.height * 0.5f) : col[i].circle.radius;

        /* Determine which edge is being crossed, if any */
        int direction = -1;

        if (py <= WALL_THICKNESS + hh)
            direction = DIR_NORTH;
        else if (py >= PLAYFIELD_H - WALL_THICKNESS - hh)
            direction = DIR_SOUTH;
        else if (px >= PLAYFIELD_W - WALL_THICKNESS - hw)
            direction = DIR_EAST;
        else if (px <= WALL_THICKNESS + hw)
            direction = DIR_WEST;

        if (direction < 0) continue;

        const RoomExit *exit = &room->exits[direction];

        /* No exit on this edge */
        if (exit->destination_room == ROOM_NONE)
        {
            /* Clamp back */
            switch (direction)
            {
                case DIR_NORTH: tf[i].position.y = WALL_THICKNESS + hh; break;
                case DIR_SOUTH: tf[i].position.y = PLAYFIELD_H - WALL_THICKNESS - hh; break;
                case DIR_EAST:  tf[i].position.x = PLAYFIELD_W - WALL_THICKNESS - hw; break;
                case DIR_WEST:  tf[i].position.x = WALL_THICKNESS + hw; break;
            }
            continue;
        }

        /* Check if position is within exit span */
        bool in_span = false;
        float perp = 0.0f; /* perpendicular coordinate */

        switch (direction)
        {
            case DIR_NORTH:
            case DIR_SOUTH:
                perp = px;
                in_span = (perp >= exit->span_min && perp <= exit->span_max);
                break;
            case DIR_EAST:
            case DIR_WEST:
                perp = py;
                in_span = (perp >= exit->span_min && perp <= exit->span_max);
                break;
            default:
                continue;
        }

        if (!in_span)
        {
            /* Not aligned with exit, clamp back */
            switch (direction)
            {
                case DIR_NORTH: tf[i].position.y = WALL_THICKNESS + hh; break;
                case DIR_SOUTH: tf[i].position.y = PLAYFIELD_H - WALL_THICKNESS - hh; break;
                case DIR_EAST:  tf[i].position.x = PLAYFIELD_W - WALL_THICKNESS - hw; break;
                case DIR_WEST:  tf[i].position.x = WALL_THICKNESS + hw; break;
            }
            continue;
        }

        /* Check if gate blocks this exit */
        if (_is_exit_blocked_by_gate(it->world, occ[i].room_id, direction))
        {
            switch (direction)
            {
                case DIR_NORTH: tf[i].position.y = WALL_THICKNESS + hh; break;
                case DIR_SOUTH: tf[i].position.y = PLAYFIELD_H - WALL_THICKNESS - hh; break;
                case DIR_EAST:  tf[i].position.x = PLAYFIELD_W - WALL_THICKNESS - hw; break;
                case DIR_WEST:  tf[i].position.x = WALL_THICKNESS + hw; break;
            }
            continue;
        }

        /* --- Transition to destination room --- */
        int dest_room = exit->destination_room;
        const RoomDef *dest = room_get(dest_room);
        if (!dest)
            continue;

        int arrival_dir = -1;
        switch (direction)
        {
            case DIR_NORTH: arrival_dir = DIR_SOUTH; break;
            case DIR_SOUTH: arrival_dir = DIR_NORTH; break;
            case DIR_EAST:  arrival_dir = DIR_WEST;  break;
            case DIR_WEST:  arrival_dir = DIR_EAST;  break;
        }

        if (arrival_dir < 0)
            continue;

        const RoomExit *dest_exit = &dest->exits[arrival_dir];
        occ[i].room_id = dest_room;

        /* Set position on arrival edge with inset, clamp perpendicular to dest exit span */
        switch (direction)
        {
            case DIR_NORTH:
                tf[i].position.y = PLAYFIELD_H - WALL_THICKNESS - hh - ROOM_TRANSITION_INSET;
                tf[i].position.x = CLAMP(perp, dest_exit->span_min, dest_exit->span_max);
                break;
            case DIR_SOUTH:
                tf[i].position.y = WALL_THICKNESS + hh + ROOM_TRANSITION_INSET;
                tf[i].position.x = CLAMP(perp, dest_exit->span_min, dest_exit->span_max);
                break;
            case DIR_EAST:
                tf[i].position.x = WALL_THICKNESS + hw + ROOM_TRANSITION_INSET;
                tf[i].position.y = CLAMP(perp, dest_exit->span_min, dest_exit->span_max);
                break;
            case DIR_WEST:
                tf[i].position.x = PLAYFIELD_W - WALL_THICKNESS - hw - ROOM_TRANSITION_INSET;
                tf[i].position.y = CLAMP(perp, dest_exit->span_min, dest_exit->span_max);
                break;
        }

        /* Dragon grace timer */
        if (ecs_has(it->world, it->entities[i], Dragon))
        {
            Dragon *d = ecs_get_mut(it->world, it->entities[i], Dragon);
            if (d) d->grace_timer = 0.2f;
        }

        /* Update cached current_room if this is the player */
        if (ecs_has(it->world, it->entities[i], Player))
        {
            gs->current_room = dest_room;
        }
    }
}

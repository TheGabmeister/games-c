#include "system.h"

#include "../components/shape.h"
#include "../components/transform.h"
#include "../components/velocity.h"
#include "../components/collider.h"
#include "../components/collision.h"
#include "../components/room_occupant.h"
#include "../components/player.h"
#include "../components/dragon.h"
#include "../components/bat.h"
#include "../components/item.h"
#include "../components/carried.h"
#include "../components/gate.h"
#include "../components/input.h"

#include "../systems/input.h"
#include "../systems/render.h"
#include "../systems/collision.h"
#include "../systems/player.h"
#include "../systems/dragon.h"
#include "../systems/bat.h"
#include "../systems/magnet.h"
#include "../systems/interaction.h"
#include "../systems/room_transition.h"
#include "../systems/win.h"
#include "../systems/fog.h"
#include "../systems/particle.h"

void system_manager_init(ecs_world_t *world)
{
    /* EcsOnLoad */
    ECS_SYSTEM(world, process_input,    EcsOnLoad,  [out] Input);

    /* EcsPreUpdate */
    ECS_SYSTEM(world, collision_clear,  EcsPreUpdate, [in] Collision);

    /* EcsOnUpdate — gameplay */
    ECS_SYSTEM(world, player_intent,    EcsOnUpdate, [in] Player, [inout] Transform, [inout] Velocity, [in] RoomOccupant);
    ECS_SYSTEM(world, dragon_ai,        EcsOnUpdate, [inout] Dragon, [inout] Transform, [inout] Velocity, [in] RoomOccupant, [in] Collider);
    ECS_SYSTEM(world, bat_ai,           EcsOnUpdate, [inout] Bat, [inout] Transform, [inout] Velocity, [inout] RoomOccupant, [in] Collider);
    ECS_SYSTEM(world, magnet_pull,      EcsOnUpdate, 0);
    ECS_SYSTEM(world, move_entities,    EcsOnUpdate, [inout] Transform, [in] Velocity, [in] RoomOccupant, [in] Collider);
    ECS_SYSTEM(world, carry_sync,       EcsOnUpdate, [in] Carried, [inout] Transform, [inout] RoomOccupant);

    /* EcsPostUpdate — collision */
    ECS_SYSTEM(world, collision_detect, EcsPostUpdate, 0);

    /* EcsOnValidate — interactions */
    ECS_SYSTEM(world, resolve_player_interactions, EcsOnValidate, 0);
    ECS_SYSTEM(world, resolve_gate_state,          EcsOnValidate, 0);
    ECS_SYSTEM(world, room_transition,             EcsOnValidate, [inout] Transform, [inout] RoomOccupant, [in] Collider);
    ECS_SYSTEM(world, check_victory,               EcsOnValidate, 0);

    /* EcsPreStore — render prep */
    ECS_SYSTEM(world, render_clear, EcsPreStore, 0);

    /* EcsOnStore — rendering */
    ECS_SYSTEM(world, render_room,      EcsOnStore, 0);
    ECS_SYSTEM(world, render_entities,  EcsOnStore, 0);
    ECS_SYSTEM(world, render_fog,       EcsOnStore, 0);
    ECS_SYSTEM(world, render_particles, EcsOnStore, 0);
    ECS_SYSTEM(world, render_colliders, EcsOnStore, [in] Collider, [in] Transform, [in] RoomOccupant);
}

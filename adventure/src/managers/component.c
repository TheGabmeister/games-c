#include "../components/transform.h"
#include "../components/input.h"
#include "../components/sprite.h"
#include "../components/velocity.h"
#include "../components/shape.h"
#include "../components/collider.h"
#include "../components/collision.h"
#include "../components/room_occupant.h"
#include "../components/player.h"
#include "../components/dragon.h"
#include "../components/bat.h"
#include "../components/item.h"
#include "../components/carryable.h"
#include "../components/carried.h"
#include "../components/gate.h"
#include "../components/glow.h"
#include "../components/particle.h"
#include "component.h"

void component_manager_init(ecs_world_t *world)
{
    ECS_COMPONENT_DEFINE(world, Transform);
    ECS_COMPONENT_DEFINE(world, Sprite);
    ECS_COMPONENT_DEFINE(world, Input);
    ECS_COMPONENT_DEFINE(world, Velocity);
    ECS_COMPONENT_DEFINE(world, Shape);
    ECS_COMPONENT_DEFINE(world, Collider);
    ECS_COMPONENT_DEFINE(world, Collision);
    ECS_COMPONENT_DEFINE(world, RoomOccupant);
    ECS_COMPONENT_DEFINE(world, Player);
    ECS_COMPONENT_DEFINE(world, Dragon);
    ECS_COMPONENT_DEFINE(world, Bat);
    ECS_COMPONENT_DEFINE(world, Item);
    ECS_COMPONENT_DEFINE(world, Carryable);
    ECS_COMPONENT_DEFINE(world, Carried);
    ECS_COMPONENT_DEFINE(world, Gate);
    ECS_COMPONENT_DEFINE(world, Glow);
    ECS_COMPONENT_DEFINE(world, ParticleSystem);

    ecs_singleton_set(world, Input, {0});
}

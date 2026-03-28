#include "../components/physics.h"
#include "../components/time.h"
#include "../components/collision.h"
#include "../components/physical.h"
#include "../components/tinted.h"

#include "../managers/physics.h"
#include "../managers/entity.h"

#include "physics.h"

#include "../helpers.h"

//==============================================================================

void physics_update(ecs_iter_t *it)
{
  Time *time = ecs_singleton_get_mut(it->world, Time);
  Physics *physics = ecs_singleton_get_mut(it->world, Physics);
  b2World_Step(physics->world, time->delta, 4);

  // Process contact begin events
  b2ContactEvents events = b2World_GetContactEvents(physics->world);
  for (int i = 0; i < events.beginCount; ++i)
  {
    b2ContactBeginTouchEvent *event = &events.beginEvents[i];
    void *udA = b2Shape_GetUserData(event->shapeIdA);
    void *udB = b2Shape_GetUserData(event->shapeIdB);
    if (udA == NULL || udB == NULL)
      continue;
    ecs_entity_t entityA = *(ecs_entity_t *)udA;
    ecs_entity_t entityB = *(ecs_entity_t *)udB;
    b2Vec2 normal = {0};
    b2Vec2 point = {0};
    if (event->manifold.pointCount > 0)
    {
      point = event->manifold.points[0].point;
      normal = event->manifold.normal;
    }
    ecs_entity_t entity = ecs_new(it->world);
    ecs_set(it->world, entity, Collision, {
      .energy = 0,
      .entities = {entityA, entityB},
      .normal = _to_vector(normal),
      .contacts = {_to_vector(point), _to_vector(point)}
    });
  }
}

//------------------------------------------------------------------------------

void physics_collide(ecs_iter_t *it)
{
  for (int i = 0; i < it->count; ++i)
  {
    ecs_delete(it->world, it->entities[i]);
  }
}

#include <flecs.h>
#include <SDL3/SDL.h>

#include "../helpers.h"
#include "../defines.h"

#include "../components/physical.h"
#include "../components/renderable.h"
#include "../components/tinted.h"
#include "../components/collision.h"
#include "../components/spatial.h"
#include "../components/animated.h"
#include "../components/stateful.h"
#include "../components/aligned.h"

#include "../managers/texture.h"
#include "../managers/entity.h"
#include "../managers/system.h"

#include "physics.h"

//==============================================================================

static void _fini(ecs_world_t *world, void *context)
{
  Physics *physics = ecs_singleton_get_mut(world, Physics);
  if (B2_IS_NON_NULL(physics->world))
    b2DestroyWorld(physics->world);
  physics->world = b2_nullWorldId;
  ecs_singleton_modified(world, Physics);
}

//------------------------------------------------------------------------------

static inline void *_store_entity(ecs_entity_t value)
{
  ecs_entity_t *pointer = SDL_malloc(sizeof(ecs_entity_t));
  assert(pointer != NULL);
  *pointer = value;
  return (void *)pointer;
}

//------------------------------------------------------------------------------

static inline ecs_entity_t _get_entity(void *pointer)
{
  return *(ecs_entity_t *)pointer;
}

//------------------------------------------------------------------------------

static inline void _delete_entity(void *pointer)
{
  assert(pointer != NULL);
  SDL_free((ecs_entity_t *)pointer);
}

//------------------------------------------------------------------------------

void _destroy_physical(ecs_iter_t *it)
{
  Physical *physical = ecs_field(it, Physical, 0);
  const Physics *physics = ecs_singleton_get(it->world, Physics);
  if (B2_IS_NULL(physics->world))
    return;
  for (int i = 0; i < it->count; ++i)
  {
    for (int j = 8; j >= 0; --j)
    {
      if (B2_IS_NON_NULL(physical[i].joints[j].joint))
      {
        b2DestroyJoint(physical[i].joints[j].joint);
        physical[i].joints[j].joint = b2_nullJointId;
      }
      if (B2_IS_NON_NULL(physical[i].joints[j].body))
      {
        b2DestroyBody(physical[i].joints[j].body);
        physical[i].joints[j].body = b2_nullBodyId;
      }
    }
    if (B2_IS_NON_NULL(physical[i].shape))
    {
      void *ud = b2Shape_GetUserData(physical[i].shape);
      if (ud != NULL)
        _delete_entity(ud);
      b2DestroyShape(physical[i].shape, true);
      physical[i].shape = b2_nullShapeId;
    }
    if (B2_IS_NON_NULL(physical[i].body))
    {
      b2DestroyBody(physical[i].body);
      physical[i].body = b2_nullBodyId;
    }
  }
}

//------------------------------------------------------------------------------

void physics_manager_init(ecs_world_t *world)
{
  ecs_atfini(world, _fini, NULL);

  b2WorldDef worldDef = b2DefaultWorldDef();
  worldDef.gravity = (b2Vec2){0.0f, 0.0f};
  b2WorldId b2world = b2CreateWorld(&worldDef);

  ecs_singleton_set(world, Physics, {.world = b2world});

  ECS_OBSERVER(world, _destroy_physical, EcsOnRemove, Physical);
}

//------------------------------------------------------------------------------

void physics_manager_set_properties(ecs_world_t *world, float gravity, float damping)
{
  const Physics *physics = ecs_singleton_get(world, Physics);
  b2World_SetGravity(physics->world, (b2Vec2){0.0f, gravity});
  // Box2D doesn't have global damping — set per body via linearDamping/angularDamping
  (void)damping;
}

//------------------------------------------------------------------------------

void physics_ball(ecs_world_t *world, ecs_entity_t parent, float mass, float radius, vector2 position)
{
  float scale = 0.25;
  position = (vector2){position.x*scale, position.y*scale};
  const Physics *physics = ecs_singleton_get(world, Physics);
  ecs_entity_t entity = ecs_new(world);

  b2BodyDef bodyDef = b2DefaultBodyDef();
  bodyDef.type = b2_dynamicBody;
  bodyDef.position = _from_vector(position);
  b2BodyId body = b2CreateBody(physics->world, &bodyDef);

  b2Circle circle = {.center = {0.0f, 0.0f}, .radius = radius * 0.9f};
  b2ShapeDef shapeDef = b2DefaultShapeDef();
  shapeDef.density = mass / (3.14159f * radius * 0.9f * radius * 0.9f);
  shapeDef.material.friction = 0.3f;
  shapeDef.material.restitution = 0.8f;
  shapeDef.userData = _store_entity(entity);
  shapeDef.enableContactEvents = true;
  b2ShapeId shape = b2CreateCircleShape(body, &shapeDef, &circle);

  SDL_Texture *texture = texture_manager_get(TEXTURE_SHEEP);
  float tex_w, tex_h;
  SDL_GetTextureSize(texture, &tex_w, &tex_h);
  ecs_set(world, entity, Physical, {.body = body, .shape = shape, .type = BODY_TYPE_BALL});
  ecs_set(world, entity, Renderable, {.texture = texture, .scale = scale * radius * 0.014, .src = (SDL_FRect){0, 0, tex_w, tex_h}});
  ecs_set(world, entity, Tinted, {.tint = (color){0,0,0,0}});
  ecs_add_pair(world, entity, EcsChildOf, parent);
}

//------------------------------------------------------------------------------

void physics_line(ecs_world_t *world, ecs_entity_t parent, vector2 from, vector2 to, float radius)
{
  const Physics *physics = ecs_singleton_get(world, Physics);
  ecs_entity_t entity = ecs_new(world);

  b2BodyDef bodyDef = b2DefaultBodyDef();
  bodyDef.type = b2_staticBody;
  bodyDef.position = (b2Vec2){0.0f, 0.0f};
  b2BodyId body = b2CreateBody(physics->world, &bodyDef);

  b2Segment segment = {.point1 = _from_vector(from), .point2 = _from_vector(to)};
  b2ShapeDef shapeDef = b2DefaultShapeDef();
  shapeDef.material.friction = 0.7f;
  shapeDef.material.restitution = 0.8f;
  shapeDef.userData = _store_entity(entity);
  shapeDef.enableContactEvents = true;
  b2ShapeId shape = b2CreateSegmentShape(body, &shapeDef, &segment);

  ecs_set(world, entity, Physical, {.body = body, .shape = shape, .type = BODY_TYPE_WALL});
  ecs_add_pair(world, entity, EcsChildOf, parent);
}

//------------------------------------------------------------------------------

void physics_box(ecs_world_t *world, ecs_entity_t parent, vector2 position)
{
  const Physics *physics = ecs_singleton_get(world, Physics);
  ecs_entity_t entity = ecs_new(world);
  float size = 2.0;

  b2BodyDef bodyDef = b2DefaultBodyDef();
  bodyDef.type = b2_staticBody;
  bodyDef.position = _from_vector(position);
  b2BodyId body = b2CreateBody(physics->world, &bodyDef);

  b2Polygon box = b2MakeBox(size, size);
  b2ShapeDef shapeDef = b2DefaultShapeDef();
  shapeDef.material.friction = 0.7f;
  shapeDef.material.restitution = 0.8f;
  shapeDef.userData = _store_entity(entity);
  shapeDef.enableContactEvents = true;
  b2ShapeId shape = b2CreatePolygonShape(body, &shapeDef, &box);

  ecs_set(world, entity, Physical, {.body = body, .shape = shape, .type = BODY_TYPE_WALL});
  ecs_add_pair(world, entity, EcsChildOf, parent);
}

//------------------------------------------------------------------------------

void physics_wedge(ecs_world_t *world, ecs_entity_t parent, vector2 position, int corner)
{
  const Physics *physics = ecs_singleton_get(world, Physics);
  ecs_entity_t entity = ecs_new(world);
  float size = 2.0;

  b2BodyDef bodyDef = b2DefaultBodyDef();
  bodyDef.type = b2_staticBody;
  bodyDef.position = (b2Vec2){0.0f, 0.0f};
  b2BodyId body = b2CreateBody(physics->world, &bodyDef);

  b2Vec2 vertices[] = {
    {position.x - size, position.y - size},
    {position.x + size, position.y - size},
    {position.x + size, position.y + size},
    {position.x - size, position.y + size}
  };
  b2Vec2 wedge[3] = {0};
  int j = 0;
  for (int i = 0; i < 4; ++i)
  {
    if (i == corner)
      continue;
    wedge[j] = vertices[i];
    ++j;
  }

  b2Hull hull = b2ComputeHull(wedge, 3);
  b2Polygon polygon = b2MakePolygon(&hull, 0.0f);
  b2ShapeDef shapeDef = b2DefaultShapeDef();
  shapeDef.material.friction = 0.7f;
  shapeDef.material.restitution = 0.8f;
  shapeDef.userData = _store_entity(entity);
  shapeDef.enableContactEvents = true;
  b2ShapeId shape = b2CreatePolygonShape(body, &shapeDef, &polygon);

  ecs_set(world, entity, Physical, {.body = body, .shape = shape, .type = BODY_TYPE_WALL});
  ecs_add_pair(world, entity, EcsChildOf, parent);
}

//==============================================================================

#ifdef DEBUG

static b2DebugDraw _debugDraw = {0};

b2DebugDraw *physics_debug_options(void)
{
  if (_debugDraw.drawShapes == false)
  {
    _debugDraw = b2DefaultDebugDraw();
    _debugDraw.drawShapes = true;
    _debugDraw.drawContacts = true;
  }
  return &_debugDraw;
}

#endif

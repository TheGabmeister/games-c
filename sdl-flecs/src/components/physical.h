#ifndef PHYSICAL_COMPONENT_H
#define PHYSICAL_COMPONENT_H

#include <flecs.h>
#include <box2d/box2d.h>

typedef enum BodyType
{
  BODY_TYPE_NONE,
  BODY_TYPE_WALL,
  BODY_TYPE_BALL,
  MAX_BODY_TYPES
} BodyType;

typedef struct Joint
{
  b2BodyId body;
  b2JointId joint;
} Joint;

typedef struct Physical
{
  BodyType type;
  b2BodyId body;
  b2ShapeId shape;
  Joint joints[9];
  int owner;
  int team;
} Physical;

ECS_COMPONENT_DECLARE(Physical);

#endif

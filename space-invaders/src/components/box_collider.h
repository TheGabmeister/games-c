#ifndef BOX_COLLIDER_COMPONENT_H
#define BOX_COLLIDER_COMPONENT_H

#include <flecs.h>

typedef struct { float w, h; } BoxCollider;

ECS_COMPONENT_DECLARE(BoxCollider);

#endif
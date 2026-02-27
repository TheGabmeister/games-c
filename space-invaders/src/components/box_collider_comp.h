#pragma once

#include <flecs.h>

typedef struct { float w, h; } BoxCollider;

extern ECS_COMPONENT_DECLARE(BoxCollider);

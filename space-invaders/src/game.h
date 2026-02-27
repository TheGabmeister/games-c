#pragma once

#include <flecs.h>
#include "components.h"

#define WINDOW_W 600
#define WINDOW_H 800

extern ECS_COMPONENT_DECLARE(Position);
extern ECS_COMPONENT_DECLARE(Size);
extern ECS_COMPONENT_DECLARE(Velocity);
extern ECS_COMPONENT_DECLARE(BoxCollider);
extern ECS_TAG_DECLARE(Projectile);
extern ECS_TAG_DECLARE(Enemy);
extern ECS_TAG_DECLARE(EnemyProjectile);
extern ECS_TAG_DECLARE(Player);

typedef struct {
    ecs_entity_t e;
    float x, y, w, h;
} ColliderBounds;

int game_run(void);

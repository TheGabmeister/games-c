#pragma once

#include <SDL3/SDL.h>
#include <flecs.h>

/* ---- Component structs ---- */

typedef struct { float x, y; }  Position;
typedef struct { float w, h; }  Size;
typedef struct { float x, y; }  Velocity;
typedef struct { float w, h; }  BoxCollider;

typedef struct {
    SDL_Texture *texture; /* NULL = draw as filled rect */
    SDL_Color    color;
} Sprite;

typedef struct { int current, max; } Health;

/* ---- ECS component declarations ---- */

extern ECS_COMPONENT_DECLARE(Position);
extern ECS_COMPONENT_DECLARE(Size);
extern ECS_COMPONENT_DECLARE(Velocity);
extern ECS_COMPONENT_DECLARE(BoxCollider);
extern ECS_COMPONENT_DECLARE(Sprite);
extern ECS_COMPONENT_DECLARE(Health);

/* ---- ECS tag declarations ---- */

extern ECS_TAG_DECLARE(Projectile);
extern ECS_TAG_DECLARE(Enemy);
extern ECS_TAG_DECLARE(EnemyProjectile);
extern ECS_TAG_DECLARE(Player);

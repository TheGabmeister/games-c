#ifndef PROJECTILE_H
#define PROJECTILE_H

#include "raylib.h"
#include "game_config.h"
#include <stdbool.h>

struct Screen;

typedef enum ProjectileType {
    PROJ_ARROW = 0,
    PROJ_BOOMERANG,
    PROJ_BOMB,
    PROJ_ROCK,
    PROJ_SPEAR,
    PROJ_DRAGON_BEAM,
    PROJ_TYPE_COUNT
} ProjectileType;

typedef enum ProjectileOwner {
    OWNER_PLAYER = 0,
    OWNER_ENEMY,
} ProjectileOwner;

typedef struct Projectile {
    ProjectileType type;
    ProjectileOwner owner;
    Vector2 pos;
    Vector2 velocity;
    Direction facing;
    bool active;
    int timer;
    float range_left;
    bool returning;
    int damage;
    int stun_frames;
} Projectile;

typedef struct ProjectileDef {
    float speed;
    int damage;
    float max_range;
    int hitbox_w;
    int hitbox_h;
    bool blocked_by_walls;
    bool blocked_by_shield_small;
    bool blocked_by_shield_large;
    void (*update)(Projectile *self, const struct Screen *screen, Vector2 player_pos, float dt);
    void (*draw)(const Projectile *self);
} ProjectileDef;

extern const ProjectileDef projectile_defs[PROJ_TYPE_COUNT];

void projectile_spawn(Projectile projs[], int *count,
                      ProjectileType type, ProjectileOwner owner,
                      Vector2 pos, Direction dir);
void projectiles_update(Projectile projs[], int *count,
                        const struct Screen *screen, Vector2 player_pos, float dt);
void projectiles_draw(const Projectile projs[], int count);
void projectiles_clear(Projectile projs[], int *count);
Rectangle projectile_hitbox(const Projectile *proj);

#endif

#ifndef ENTITY_H
#define ENTITY_H

#include "common.h"

typedef struct Entity Entity;
typedef struct Game Game;

typedef struct {
    void (*update)(Entity *self, Game *game);
    void (*draw)(Entity *self, float camera_x);
    void (*touch)(Entity *self, Entity *other, Game *game);
    void (*stomped)(Entity *self, Entity *mario, Game *game);
    void (*hit_by_fire)(Entity *self, Game *game);
    void (*hit_by_shell)(Entity *self, Game *game);
    void (*hit_by_star)(Entity *self, Game *game);
    void (*bumped)(Entity *self, Game *game);
    void (*kill)(Entity *self, Game *game);
} EntityVtab;

struct Entity {
    EntityType type;
    const EntityVtab *vtab;

    // Position & size
    float x, y;
    float w, h;

    // Velocity
    float vx, vy;

    // State
    Direction facing;
    bool on_ground;
    bool active;

    // Collision flags
    bool stompable;
    bool damages_mario;
    bool fire_immune;
    bool shell_killable;
    bool star_killable;
    bool destructible;

    // Movement/state flags
    bool self_moving;
    bool dead_falling;

    // Mario-specific state
    MarioPower power;
    bool jumping;
    bool running;
    float invincible_timer;
    bool star_active;
    float star_timer;

    // Animation
    float anim_timer;
    int anim_frame;

    // Generic state timer
    float state_timer;
    int state_val;
};

Entity *entity_alloc(Entity entities[MAX_ENTITIES]);
void entity_deactivate(Entity *e);
bool entity_overlap(Entity *a, Entity *b);

#endif

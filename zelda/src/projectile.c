#include "projectile.h"
#include "tilemap.h"
#include "textures.h"
#include "raymath.h"
#include <math.h>

static void dir_to_velocity(Direction dir, float speed, Vector2 *vel) {
    switch (dir) {
        case DIR_N: *vel = (Vector2){ 0, -speed }; break;
        case DIR_S: *vel = (Vector2){ 0,  speed }; break;
        case DIR_E: *vel = (Vector2){ speed, 0 }; break;
        case DIR_W: *vel = (Vector2){-speed, 0 }; break;
        default:    *vel = (Vector2){ 0, 0 };      break;
    }
}

static bool out_of_bounds(Vector2 pos) {
    return pos.x < -TILE_SIZE || pos.x > SCREEN_TILES_X * TILE_SIZE ||
           pos.y < PLAY_AREA_Y - TILE_SIZE ||
           pos.y > PLAY_AREA_Y + SCREEN_TILES_Y * TILE_SIZE;
}

// --- Linear projectile (arrow, rock, spear) ---

static void linear_update(Projectile *self, const Screen *screen,
                           Vector2 player_pos, float dt) {
    (void)player_pos;
    self->pos.x += self->velocity.x * dt;
    self->pos.y += self->velocity.y * dt;
    self->range_left -= self->velocity.x != 0 ?
        fabsf(self->velocity.x * dt) : fabsf(self->velocity.y * dt);

    if (out_of_bounds(self->pos)) { self->active = false; return; }
    if (self->range_left <= 0)    { self->active = false; return; }

    if (projectile_defs[self->type].blocked_by_walls) {
        Rectangle hb = projectile_hitbox(self);
        if (screen_tile_blocked(screen, hb)) {
            self->active = false;
        }
    }
}

static void linear_draw(const Projectile *self) {
    Color c;
    int w = 16, h = 16;
    switch (self->type) {
        case PROJ_ARROW: c = WHITE;  w = (self->facing == DIR_E || self->facing == DIR_W) ? 32 : 8;
                                     h = (self->facing == DIR_N || self->facing == DIR_S) ? 32 : 8; break;
        case PROJ_ROCK:  c = GRAY;   w = 16; h = 16; break;
        case PROJ_SPEAR: c = BROWN;  w = (self->facing == DIR_E || self->facing == DIR_W) ? 32 : 8;
                                     h = (self->facing == DIR_N || self->facing == DIR_S) ? 32 : 8; break;
        default:         c = WHITE;  break;
    }
    int ox = (TILE_SIZE - w) / 2;
    int oy = (TILE_SIZE - h) / 2;
    DrawRectangle((int)self->pos.x + ox, (int)self->pos.y + oy, w, h, c);
}

// --- Boomerang ---

static void boomerang_update(Projectile *self, const Screen *screen,
                              Vector2 player_pos, float dt) {
    if (!self->returning) {
        self->pos.x += self->velocity.x * dt;
        self->pos.y += self->velocity.y * dt;
        self->range_left -= BOOMERANG_SPEED * dt;

        if (self->range_left <= 0 || out_of_bounds(self->pos)) {
            self->returning = true;
        } else {
            Rectangle hb = projectile_hitbox(self);
            if (screen_tile_blocked(screen, hb)) {
                self->returning = true;
            }
        }
    } else {
        float dx = player_pos.x - self->pos.x;
        float dy = player_pos.y - self->pos.y;
        float dist = sqrtf(dx * dx + dy * dy);
        if (dist < 16.0f) {
            self->active = false;
            return;
        }
        float nx = dx / dist;
        float ny = dy / dist;
        self->pos.x += nx * BOOMERANG_SPEED * dt;
        self->pos.y += ny * BOOMERANG_SPEED * dt;
    }
}

static void boomerang_draw(const Projectile *self) {
    int size = 20;
    int ox = (TILE_SIZE - size) / 2;
    int oy = (TILE_SIZE - size) / 2;
    Color c = self->returning ? BLUE : SKYBLUE;
    DrawRectangle((int)self->pos.x + ox, (int)self->pos.y + oy, size, size, c);
}

// --- Bomb ---

static void bomb_update(Projectile *self, const Screen *screen,
                         Vector2 player_pos, float dt) {
    (void)screen;
    (void)player_pos;
    (void)dt;
    if (self->timer > 0) {
        self->timer--;
    }
}

static void bomb_draw(const Projectile *self) {
    int size = 24;
    int ox = (TILE_SIZE - size) / 2;
    int oy = (TILE_SIZE - size) / 2;
    bool flash = self->timer < 60 && (self->timer / 4) % 2 == 0;
    Color c = flash ? WHITE : DARKGRAY;
    DrawRectangle((int)self->pos.x + ox, (int)self->pos.y + oy, size, size, c);
    DrawRectangleLines((int)self->pos.x + ox, (int)self->pos.y + oy, size, size, BLACK);
}

// --- Def table ---

const ProjectileDef projectile_defs[PROJ_TYPE_COUNT] = {
    [PROJ_ARROW] = {
        ARROW_SPEED, ARROW_DAMAGE, ARROW_MAX_RANGE,
        true, true, true,
        linear_update, linear_draw
    },
    [PROJ_BOOMERANG] = {
        BOOMERANG_SPEED, 0, BOOMERANG_MAX_RANGE,
        false, false, false,
        boomerang_update, boomerang_draw
    },
    [PROJ_BOMB] = {
        0, BOMB_DAMAGE, 0,
        false, false, false,
        bomb_update, bomb_draw
    },
    [PROJ_ROCK] = {
        ENEMY_ROCK_SPEED, 1, ARROW_MAX_RANGE,
        true, true, false,
        linear_update, linear_draw
    },
    [PROJ_SPEAR] = {
        ENEMY_SPEAR_SPEED, 1, ARROW_MAX_RANGE,
        true, true, false,
        linear_update, linear_draw
    },
};

// --- API ---

void projectile_spawn(Projectile projs[], int *count,
                      ProjectileType type, ProjectileOwner owner,
                      Vector2 pos, Direction dir) {
    if (*count >= MAX_PROJECTILES) return;
    const ProjectileDef *def = &projectile_defs[type];
    Projectile *p = &projs[(*count)++];
    *p = (Projectile){0};
    p->type = type;
    p->owner = owner;
    p->pos = pos;
    p->facing = dir;
    p->active = true;
    p->damage = def->damage;
    p->range_left = def->max_range;
    dir_to_velocity(dir, def->speed, &p->velocity);

    if (type == PROJ_BOOMERANG) {
        p->stun_frames = BOOMERANG_STUN_FRAMES;
    }
    if (type == PROJ_BOMB) {
        p->timer = BOMB_FUSE_FRAMES;
        p->velocity = (Vector2){0, 0};
    }
}

void projectiles_update(Projectile projs[], int *count,
                        const Screen *screen, Vector2 player_pos, float dt) {
    for (int i = 0; i < *count; i++) {
        if (!projs[i].active) continue;
        projectile_defs[projs[i].type].update(&projs[i], screen, player_pos, dt);
    }

    // compact: remove trailing inactive
    while (*count > 0 && !projs[*count - 1].active) {
        (*count)--;
    }
}

void projectiles_draw(const Projectile projs[], int count) {
    for (int i = 0; i < count; i++) {
        if (!projs[i].active) continue;
        projectile_defs[projs[i].type].draw(&projs[i]);
    }
}

void projectiles_clear(Projectile projs[], int *count) {
    for (int i = 0; i < *count; i++) {
        projs[i].active = false;
    }
    *count = 0;
}

Rectangle projectile_hitbox(const Projectile *proj) {
    int w, h;
    switch (proj->type) {
        case PROJ_ARROW:
        case PROJ_SPEAR:
            if (proj->facing == DIR_E || proj->facing == DIR_W) { w = 32; h = 8; }
            else { w = 8; h = 32; }
            break;
        case PROJ_BOOMERANG: w = 20; h = 20; break;
        case PROJ_BOMB:      w = 24; h = 24; break;
        case PROJ_ROCK:      w = 16; h = 16; break;
        default:             w = 16; h = 16; break;
    }
    int ox = (TILE_SIZE - w) / 2;
    int oy = (TILE_SIZE - h) / 2;
    return (Rectangle){ proj->pos.x + ox, proj->pos.y + oy, (float)w, (float)h };
}

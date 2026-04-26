#include "hammer_bro.h"
#include "../game.h"
#include "../sounds.h"

// --- Hammer projectile ---
// Hammers arc through the air. Indestructible. Damages Mario on contact.

static void hammer_update(Entity *self, Game *game) {
    float dt = GetFrameTime();
    self->vy += GRAVITY * dt;
    self->x += self->vx * dt;
    self->y += self->vy * dt;
    self->anim_timer += dt;

    if (self->y > game->level.height * TILE_SIZE + 100 ||
        self->x < game->camera_x - WINDOW_WIDTH ||
        self->x > game->camera_x + WINDOW_WIDTH * 2)
        entity_deactivate(self);
}

static void hammer_draw(Entity *self, float camera_x) {
    float dx = self->x - camera_x;
    float dy = self->y;
    float angle = self->anim_timer * 720.0f;
    Vector2 center = {dx + self->w / 2, dy + self->h / 2};
    // Handle (brown rectangle)
    DrawRectanglePro(
        (Rectangle){center.x, center.y, 8, 20},
        (Vector2){4, 10}, angle, COLOR_HAMMER);
    // Head (darker rectangle)
    DrawRectanglePro(
        (Rectangle){center.x, center.y - 8, 16, 10},
        (Vector2){8, 13}, angle, (Color){100, 70, 40, 255});
}

static const EntityVtab hammer_vtab = {
    .update = hammer_update,
    .draw   = hammer_draw,
};

void spawn_hammer_projectile(Entity entities[MAX_ENTITIES], float x, float y, Direction dir) {
    Entity *e = entity_alloc(entities);
    if (!e) return;
    e->type = ENT_HAMMER;
    e->vtab = &hammer_vtab;
    e->x = x;
    e->y = y;
    e->w = HAMMER_W;
    e->h = HAMMER_HEIGHT;
    e->vx = (dir == DIR_LEFT) ? -HAMMER_SPEED_X : HAMMER_SPEED_X;
    e->vy = HAMMER_SPEED_Y;
    e->facing = dir;
    e->active = true;
    e->damages_mario = true;
    e->fire_immune = true;
    e->self_moving = true;
}

// --- Hammer Bro ---
// state_val: 0 = normal
// anim_timer: used for animation
// state_timer: shared timer for jump and throw cooldowns
// Uses jumping field to track jump cooldown, running field for throw cooldown tracking

static void hammer_bro_update(Entity *self, Game *game) {
    float dt = GetFrameTime();
    Entity *mario = &game->entities[game->mario];

    self->vy += GRAVITY * dt;
    if (self->vy > MAX_FALL_SPEED) self->vy = MAX_FALL_SPEED;

    // Face Mario
    if (mario->x < self->x)
        self->facing = DIR_LEFT;
    else
        self->facing = DIR_RIGHT;

    // Pace back and forth on platform
    self->state_timer += dt;
    float pace = sinf(self->state_timer * 1.5f);
    self->vx = pace * HAMMER_BRO_SPEED;

    // Jump periodically between platform levels (uses invincible_timer as jump cooldown)
    self->invincible_timer += dt;
    if (self->on_ground && self->invincible_timer >= HAMMER_BRO_JUMP_INTERVAL) {
        self->vy = HAMMER_BRO_JUMP_VEL;
        self->on_ground = false;
        self->invincible_timer = 0;
    }

    // Throw hammers periodically
    self->anim_timer += dt;
    if (self->anim_timer >= HAMMER_BRO_THROW_INTERVAL) {
        self->anim_timer = 0;
        float hx = self->x + self->w / 2;
        float hy = self->y;
        spawn_hammer_projectile(game->entities, hx, hy, self->facing);
    }

    // Advance toward Mario if he's close
    float dist = fabsf(mario->x - self->x);
    if (dist < HAMMER_BRO_ADVANCE_DIST) {
        float dir = (mario->x < self->x) ? -1.0f : 1.0f;
        self->vx += dir * HAMMER_BRO_SPEED * 0.5f;
    }

    // Despawn offscreen
    if (self->x < game->camera_x - WINDOW_WIDTH || self->x > game->camera_x + WINDOW_WIDTH * 2)
        entity_deactivate(self);
}

static void hammer_bro_draw(Entity *self, float camera_x) {
    float dx = self->x - camera_x;
    float dy = self->y;

    // Body
    DrawRectangle((int)dx, (int)dy, (int)self->w, (int)self->h, COLOR_HAMMER_BRO);
    // Helmet
    DrawRectangle((int)(dx + 4), (int)(dy - 4), (int)(self->w - 8), 12, COLOR_HAMMER);
    // Eyes
    float eye_x = (self->facing == DIR_LEFT) ? dx + self->w * 0.3f : dx + self->w * 0.7f;
    DrawCircle((int)eye_x, (int)(dy + 16), 4, WHITE);
    DrawCircle((int)eye_x, (int)(dy + 16), 2, BLACK);
    // Arm (throwing pose)
    float arm_x = (self->facing == DIR_LEFT) ? dx - 4 : dx + self->w - 4;
    DrawRectangle((int)arm_x, (int)(dy + 12), 8, 16, COLOR_HAMMER_BRO);
}

static void hammer_bro_stomped(Entity *self, Entity *mario, Game *game) {
    (void)mario;
    game->score += SCORE_KOOPA_STOMP;
    sound_play(SND_STOMP);
    entity_deactivate(self);
}

static void hammer_bro_hit_by_fire(Entity *self, Game *game) {
    game->score += SCORE_FIREBALL_KILL;
    self->vy = -400.0f;
    self->damages_mario = false;
    self->stompable = false;
    self->state_val = 2;
    self->dead_falling = true;
}

static void hammer_bro_hit_by_shell(Entity *self, Game *game) {
    game->score += SCORE_SHELL_KILL;
    self->vy = -400.0f;
    self->damages_mario = false;
    self->stompable = false;
    self->state_val = 2;
    self->dead_falling = true;
}

static void hammer_bro_hit_by_star(Entity *self, Game *game) {
    game->score += SCORE_KOOPA_STOMP;
    self->vy = -400.0f;
    self->damages_mario = false;
    self->stompable = false;
    self->state_val = 2;
    self->dead_falling = true;
}

static void hammer_bro_bumped(Entity *self, Game *game) {
    game->score += SCORE_KOOPA_STOMP;
    self->vy = -400.0f;
    self->vx = 0;
    self->damages_mario = false;
    self->stompable = false;
    self->state_val = 2;
    self->dead_falling = true;
}

static const EntityVtab hammer_bro_vtab = {
    .update       = hammer_bro_update,
    .draw         = hammer_bro_draw,
    .stomped      = hammer_bro_stomped,
    .hit_by_fire  = hammer_bro_hit_by_fire,
    .hit_by_shell = hammer_bro_hit_by_shell,
    .hit_by_star  = hammer_bro_hit_by_star,
    .bumped       = hammer_bro_bumped,
};

void spawn_hammer_bro(Entity entities[MAX_ENTITIES], float x, float y, int extra) {
    (void)extra;
    Entity *e = entity_alloc(entities);
    if (!e) return;

    e->type = ENT_HAMMER_BRO;
    e->vtab = &hammer_bro_vtab;
    e->x = x;
    e->y = y - (KOOPA_HEIGHT - TILE_SIZE);
    e->w = ENEMY_W;
    e->h = KOOPA_HEIGHT;
    e->vx = 0;
    e->facing = DIR_LEFT;
    e->active = true;

    e->stompable = true;
    e->damages_mario = true;
    e->shell_killable = true;
    e->star_killable = true;
    e->destructible = true;
}

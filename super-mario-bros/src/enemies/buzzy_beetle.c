#include "buzzy_beetle.h"
#include "koopa.h"
#include "../game.h"
#include "../sounds.h"

// Buzzy Beetle: walks like Goomba, fireproof, enters shell when stomped.
// The shell it produces is also fireproof.

static void buzzy_update(Entity *self, Game *game) {
    float dt = GetFrameTime();
    self->vy += GRAVITY * dt;
    if (self->vy > MAX_FALL_SPEED) self->vy = MAX_FALL_SPEED;
    self->anim_timer += dt;

    if (self->x < game->camera_x - WINDOW_WIDTH || self->x > game->camera_x + WINDOW_WIDTH * 2)
        entity_deactivate(self);
}

static void buzzy_draw(Entity *self, float camera_x) {
    float dx = self->x - camera_x;
    float dy = self->y;
    // Dome shell
    DrawEllipse((int)(dx + self->w / 2), (int)(dy + self->h * 0.4f),
                (int)(self->w / 2), (int)(self->h * 0.4f), COLOR_BUZZY);
    // Body underneath
    DrawRectangle((int)(dx + 4), (int)(dy + self->h * 0.4f),
                  (int)(self->w - 8), (int)(self->h * 0.6f), COLOR_BUZZY);
    // Eyes
    float eye_x = (self->facing == DIR_LEFT) ? dx + self->w * 0.3f : dx + self->w * 0.7f;
    DrawCircle((int)eye_x, (int)(dy + self->h * 0.35f), 4, WHITE);
    DrawCircle((int)eye_x, (int)(dy + self->h * 0.35f), 2, BLACK);
}

static void buzzy_stomped(Entity *self, Entity *mario, Game *game) {
    (void)mario;
    game->score += SCORE_KOOPA_STOMP;
    sound_play(SND_STOMP);
    // Spawn a fireproof shell
    spawn_shell(game->entities, self->x, self->y + (self->h - SHELL_H), 0);
    // Make the shell fireproof — find it (just spawned, will be the last ENT_SHELL with vx==0)
    for (int i = MAX_ENTITIES - 1; i >= 0; i--) {
        Entity *e = &game->entities[i];
        if (e->type == ENT_SHELL && fabsf(e->vx) < 1.0f &&
            fabsf(e->x - self->x) < TILE_SIZE) {
            e->fire_immune = true;
            break;
        }
    }
    entity_deactivate(self);
}

static void buzzy_hit_by_fire(Entity *self, Game *game) {
    (void)self;
    (void)game;
    // Fireproof — fireballs do nothing
}

static void buzzy_hit_by_shell(Entity *self, Game *game) {
    game->score += SCORE_SHELL_KILL;
    self->vy = -400.0f;
    self->damages_mario = false;
    self->stompable = false;
    self->state_val = 2;
    self->dead_falling = true;
}

static void buzzy_hit_by_star(Entity *self, Game *game) {
    game->score += SCORE_GOOMBA_STOMP;
    self->vy = -400.0f;
    self->damages_mario = false;
    self->stompable = false;
    self->state_val = 2;
    self->dead_falling = true;
}

static void buzzy_bumped(Entity *self, Game *game) {
    game->score += SCORE_GOOMBA_STOMP;
    self->vy = -400.0f;
    self->vx = 0;
    self->damages_mario = false;
    self->stompable = false;
    self->state_val = 2;
    self->dead_falling = true;
}

static void buzzy_touch(Entity *self, Entity *other, Game *game) {
    (void)game;
    if (other->type != ENT_MARIO && other->type != ENT_FIREBALL) {
        self->vx = -self->vx;
        self->facing = (self->vx < 0) ? DIR_LEFT : DIR_RIGHT;
    }
}

static const EntityVtab buzzy_vtab = {
    .update       = buzzy_update,
    .draw         = buzzy_draw,
    .touch        = buzzy_touch,
    .stomped      = buzzy_stomped,
    .hit_by_fire  = buzzy_hit_by_fire,
    .hit_by_shell = buzzy_hit_by_shell,
    .hit_by_star  = buzzy_hit_by_star,
    .bumped       = buzzy_bumped,
};

void spawn_buzzy_beetle(Entity entities[MAX_ENTITIES], float x, float y, int extra) {
    (void)extra;
    Entity *e = entity_alloc(entities);
    if (!e) return;

    e->type = ENT_BUZZY_BEETLE;
    e->vtab = &buzzy_vtab;
    e->x = x;
    e->y = y;
    e->w = ENEMY_W;
    e->h = ENEMY_H;
    e->vx = -BUZZY_SPEED;
    e->facing = DIR_LEFT;
    e->active = true;

    e->stompable = true;
    e->damages_mario = true;
    e->fire_immune = true;
    e->shell_killable = true;
    e->star_killable = true;
    e->destructible = true;
}

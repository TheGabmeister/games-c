#include "goomba.h"
#include "../game.h"
#include "../sounds.h"

static void goomba_update(Entity *self, Game *game) {
    float dt = GetFrameTime();

    if (self->state_val == 1) {
        self->state_timer -= dt;
        if (self->state_timer <= 0)
            entity_deactivate(self);
        return;
    }

    self->vy += GRAVITY * dt;
    if (self->vy > MAX_FALL_SPEED) self->vy = MAX_FALL_SPEED;

    self->anim_timer += dt;

    // Despawn if far offscreen
    if (self->x < game->camera_x - WINDOW_WIDTH || self->x > game->camera_x + WINDOW_WIDTH * 2)
        entity_deactivate(self);
}

static void goomba_draw(Entity *self, float camera_x) {
    float dx = self->x - camera_x;
    float dy = self->y;

    // Squished: draw flat
    if (self->state_val == 1) {
        DrawRectangle((int)dx, (int)(dy + self->h - 12), (int)self->w, 12, COLOR_GOOMBA);
        return;
    }

    DrawRectangle((int)dx, (int)dy, (int)self->w, (int)self->h, COLOR_GOOMBA);
    // Eyes
    float eye_y = dy + self->h * 0.25f;
    DrawCircle((int)(dx + self->w * 0.3f), (int)eye_y, 4, WHITE);
    DrawCircle((int)(dx + self->w * 0.7f), (int)eye_y, 4, WHITE);
    DrawCircle((int)(dx + self->w * 0.3f), (int)eye_y, 2, BLACK);
    DrawCircle((int)(dx + self->w * 0.7f), (int)eye_y, 2, BLACK);
}

static void goomba_stomped(Entity *self, Entity *mario, Game *game) {
    (void)mario;
    sound_play(SND_STOMP);
    game->score += SCORE_GOOMBA_STOMP;
    // Show squished briefly then deactivate
    self->state_val = 1;
    self->vx = 0;
    self->vy = 0;
    self->state_timer = 0.4f;
    self->damages_mario = false;
    self->stompable = false;
}

static void goomba_touch(Entity *self, Entity *other, Game *game) {
    (void)self;
    (void)game;
    // Side contact with another entity: reverse direction
    if (other->type != ENT_MARIO && other->type != ENT_FIREBALL) {
        self->vx = -self->vx;
        self->facing = (self->vx < 0) ? DIR_LEFT : DIR_RIGHT;
    }
}

static void goomba_kill(Entity *self, Game *game) {
    (void)game;
    entity_deactivate(self);
}

static void goomba_hit_by_fire(Entity *self, Game *game) {
    game->score += SCORE_FIREBALL_KILL;
    // Flip upside down and fall
    self->vy = -400.0f;
    self->damages_mario = false;
    self->stompable = false;
    self->state_val = 2;
    self->dead_falling = true; // dead-falling
}

static void goomba_hit_by_shell(Entity *self, Game *game) {
    game->score += SCORE_SHELL_KILL;
    self->vy = -400.0f;
    self->damages_mario = false;
    self->stompable = false;
    self->state_val = 2;
    self->dead_falling = true;
}

static void goomba_hit_by_star(Entity *self, Game *game) {
    game->score += SCORE_GOOMBA_STOMP;
    self->vy = -400.0f;
    self->damages_mario = false;
    self->stompable = false;
    self->state_val = 2;
    self->dead_falling = true;
}

static void goomba_bumped(Entity *self, Game *game) {
    game->score += SCORE_GOOMBA_STOMP;
    self->vy = -400.0f;
    self->vx = 0;
    self->damages_mario = false;
    self->stompable = false;
    self->state_val = 2;
    self->dead_falling = true;
}

static const EntityVtab goomba_vtab = {
    .update      = goomba_update,
    .draw        = goomba_draw,
    .touch       = goomba_touch,
    .stomped     = goomba_stomped,
    .hit_by_fire = goomba_hit_by_fire,
    .hit_by_shell = goomba_hit_by_shell,
    .hit_by_star = goomba_hit_by_star,
    .bumped      = goomba_bumped,
    .kill        = goomba_kill,
};

void spawn_goomba(Entity entities[MAX_ENTITIES], float x, float y, int extra) {
    (void)extra;
    Entity *e = entity_alloc(entities);
    if (!e) return;

    e->type = ENT_GOOMBA;
    e->vtab = &goomba_vtab;
    e->x = x;
    e->y = y;
    e->w = ENEMY_W;
    e->h = ENEMY_H;
    e->vx = -GOOMBA_SPEED;
    e->facing = DIR_LEFT;
    e->active = true;

    e->stompable = true;
    e->damages_mario = true;
    e->fire_immune = false;
    e->shell_killable = true;
    e->star_killable = true;
    e->destructible = true;
}

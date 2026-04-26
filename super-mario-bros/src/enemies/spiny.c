#include "spiny.h"
#include "../game.h"
#include "../sounds.h"

// --- Spiny (walking enemy, NOT stompable) ---
// state_val: 0 = walking, 2 = dead-falling

static void spiny_update(Entity *self, Game *game) {
    float dt = GetFrameTime();
    self->vy += GRAVITY * dt;
    if (self->vy > MAX_FALL_SPEED) self->vy = MAX_FALL_SPEED;
    self->anim_timer += dt;

    if (self->x < game->camera_x - WINDOW_WIDTH || self->x > game->camera_x + WINDOW_WIDTH * 2)
        entity_deactivate(self);
}

static void spiny_draw(Entity *self, float camera_x) {
    float dx = self->x - camera_x;
    float dy = self->y;
    // Body
    DrawRectangle((int)dx, (int)(dy + 8), (int)self->w, (int)(self->h - 8), COLOR_SPINY);
    // Spikes on top
    for (int i = 0; i < 4; i++) {
        float sx = dx + 6 + i * 12;
        DrawTriangle(
            (Vector2){sx, dy + 8},
            (Vector2){sx + 6, dy - 2},
            (Vector2){sx + 12, dy + 8},
            COLOR_SPINY);
    }
    // Eyes
    float eye_x = (self->facing == DIR_LEFT) ? dx + self->w * 0.3f : dx + self->w * 0.7f;
    DrawCircle((int)eye_x, (int)(dy + 20), 4, WHITE);
    DrawCircle((int)eye_x, (int)(dy + 20), 2, BLACK);
}

static void spiny_touch(Entity *self, Entity *other, Game *game) {
    (void)game;
    if (other->type != ENT_MARIO && other->type != ENT_FIREBALL) {
        self->vx = -self->vx;
        self->facing = (self->vx < 0) ? DIR_LEFT : DIR_RIGHT;
    }
}

static void spiny_hit_by_fire(Entity *self, Game *game) {
    game->score += SCORE_FIREBALL_KILL;
    self->vy = -400.0f;
    self->damages_mario = false;
    self->stompable = false;
    self->state_val = 2;
    self->dead_falling = true;
}

static void spiny_hit_by_shell(Entity *self, Game *game) {
    game->score += SCORE_SHELL_KILL;
    self->vy = -400.0f;
    self->damages_mario = false;
    self->stompable = false;
    self->state_val = 2;
    self->dead_falling = true;
}

static void spiny_hit_by_star(Entity *self, Game *game) {
    game->score += SCORE_GOOMBA_STOMP;
    self->vy = -400.0f;
    self->damages_mario = false;
    self->stompable = false;
    self->state_val = 2;
    self->dead_falling = true;
}

static void spiny_bumped(Entity *self, Game *game) {
    game->score += SCORE_GOOMBA_STOMP;
    self->vy = -400.0f;
    self->vx = 0;
    self->damages_mario = false;
    self->stompable = false;
    self->state_val = 2;
    self->dead_falling = true;
}

static const EntityVtab spiny_vtab = {
    .update       = spiny_update,
    .draw         = spiny_draw,
    .touch        = spiny_touch,
    .hit_by_fire  = spiny_hit_by_fire,
    .hit_by_shell = spiny_hit_by_shell,
    .hit_by_star  = spiny_hit_by_star,
    .bumped       = spiny_bumped,
};

void spawn_spiny(Entity entities[MAX_ENTITIES], float x, float y, int extra) {
    (void)extra;
    Entity *e = entity_alloc(entities);
    if (!e) return;

    e->type = ENT_SPINY;
    e->vtab = &spiny_vtab;
    e->x = x;
    e->y = y;
    e->w = ENEMY_W;
    e->h = ENEMY_H;
    e->vx = -SPINY_SPEED;
    e->facing = DIR_LEFT;
    e->active = true;

    e->stompable = false;
    e->damages_mario = true;
    e->shell_killable = true;
    e->star_killable = true;
    e->destructible = true;
}

// --- Spiny Egg (thrown by Lakitu, hatches into Spiny on ground contact) ---
// Uses ENT_SPINY with anim_frame = 1 to mark as egg

static void egg_update(Entity *self, Game *game) {
    float dt = GetFrameTime();
    self->vy += GRAVITY * dt;
    if (self->vy > MAX_FALL_SPEED) self->vy = MAX_FALL_SPEED;

    // Hatch when landing
    if (self->on_ground) {
        self->vtab = &spiny_vtab;
        self->anim_frame = 0;
        self->vx = -SPINY_SPEED;
        self->facing = DIR_LEFT;
        // Decide direction based on Mario position
        Entity *mario = &game->entities[game->mario];
        if (mario->x > self->x) {
            self->vx = SPINY_SPEED;
            self->facing = DIR_RIGHT;
        }
        return;
    }

    self->anim_timer += dt;

    if (self->x < game->camera_x - WINDOW_WIDTH || self->x > game->camera_x + WINDOW_WIDTH * 2)
        entity_deactivate(self);
}

static void egg_draw(Entity *self, float camera_x) {
    float dx = self->x - camera_x;
    float dy = self->y;
    float spin = self->anim_timer * 360.0f;
    // Rotating egg shape
    DrawEllipse((int)(dx + self->w / 2), (int)(dy + self->h / 2), 14, 10, COLOR_SPINY_EGG);
    // Spots
    float spot_x = dx + self->w / 2 + sinf(spin * 0.017f) * 6;
    DrawCircle((int)spot_x, (int)(dy + self->h / 2), 3, COLOR_SPINY);
}

static const EntityVtab egg_vtab = {
    .update       = egg_update,
    .draw         = egg_draw,
    .hit_by_fire  = spiny_hit_by_fire,
    .hit_by_shell = spiny_hit_by_shell,
    .hit_by_star  = spiny_hit_by_star,
};

void spawn_spiny_egg(Entity entities[MAX_ENTITIES], float x, float y, float vx, float vy) {
    Entity *e = entity_alloc(entities);
    if (!e) return;

    e->type = ENT_SPINY;
    e->vtab = &egg_vtab;
    e->x = x;
    e->y = y;
    e->w = ENEMY_W;
    e->h = ENEMY_H;
    e->vx = vx;
    e->vy = vy;
    e->active = true;
    e->anim_frame = 1;

    e->stompable = false;
    e->damages_mario = true;
    e->shell_killable = true;
    e->star_killable = true;
    e->destructible = true;
}

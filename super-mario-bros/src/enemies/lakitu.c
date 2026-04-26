#include "lakitu.h"
#include "spiny.h"
#include "../game.h"
#include "../sounds.h"

// Lakitu flies on a cloud at the top of the screen, following Mario horizontally,
// and throws Spiny Eggs at regular intervals.
// state_timer: throw cooldown
// invincible_timer: unused (available)

static void lakitu_update(Entity *self, Game *game) {
    float dt = GetFrameTime();
    Entity *mario = &game->entities[game->mario];

    // Follow Mario horizontally with slight offset
    float target_x = mario->x + 60.0f;
    float diff = target_x - self->x;
    if (fabsf(diff) > 4.0f) {
        self->vx = (diff > 0) ? LAKITU_SPEED : -LAKITU_SPEED;
        if (fabsf(diff) < LAKITU_SPEED * dt)
            self->vx = diff / dt;
    } else {
        self->vx = 0;
    }
    self->x += self->vx * dt;
    self->facing = (mario->x < self->x) ? DIR_LEFT : DIR_RIGHT;

    // Stay at fixed height relative to camera top
    float target_y = game->camera_x > 0 ? LAKITU_FLY_Y : LAKITU_FLY_Y;
    self->y = target_y;

    // Throw Spiny Eggs
    self->state_timer += dt;
    if (self->state_timer >= LAKITU_THROW_INTERVAL) {
        self->state_timer = 0;
        float egg_x = self->x + self->w / 2;
        float egg_y = self->y + self->h;
        float dir = (mario->x < self->x) ? -1.0f : 1.0f;
        spawn_spiny_egg(game->entities, egg_x, egg_y, dir * LAKITU_EGG_VX, LAKITU_EGG_VY);
    }

    self->anim_timer += dt;
}

static void lakitu_draw(Entity *self, float camera_x) {
    float dx = self->x - camera_x;
    float dy = self->y;

    // Cloud
    DrawEllipse((int)(dx + self->w / 2), (int)(dy + self->h - 8), 30, 16, COLOR_LAKITU_CLOUD);
    DrawEllipse((int)(dx + self->w / 2 - 10), (int)(dy + self->h - 16), 18, 14, COLOR_LAKITU_CLOUD);
    DrawEllipse((int)(dx + self->w / 2 + 10), (int)(dy + self->h - 16), 18, 14, COLOR_LAKITU_CLOUD);
    // Body on cloud
    DrawRectangle((int)(dx + 8), (int)(dy + 4), (int)(self->w - 16), (int)(self->h - 20), COLOR_LAKITU);
    // Eyes
    float eye_x = (self->facing == DIR_LEFT) ? dx + self->w * 0.35f : dx + self->w * 0.65f;
    DrawCircle((int)eye_x, (int)(dy + 14), 4, WHITE);
    DrawCircle((int)eye_x, (int)(dy + 14), 2, BLACK);
}

static void lakitu_stomped(Entity *self, Entity *mario, Game *game) {
    (void)mario;
    game->score += SCORE_KOOPA_STOMP;
    sound_play(SND_STOMP);
    entity_deactivate(self);
}

static void lakitu_hit_by_fire(Entity *self, Game *game) {
    game->score += SCORE_FIREBALL_KILL;
    entity_deactivate(self);
}

static void lakitu_hit_by_shell(Entity *self, Game *game) {
    game->score += SCORE_SHELL_KILL;
    entity_deactivate(self);
}

static void lakitu_hit_by_star(Entity *self, Game *game) {
    game->score += SCORE_KOOPA_STOMP;
    entity_deactivate(self);
}

static const EntityVtab lakitu_vtab = {
    .update       = lakitu_update,
    .draw         = lakitu_draw,
    .stomped      = lakitu_stomped,
    .hit_by_fire  = lakitu_hit_by_fire,
    .hit_by_shell = lakitu_hit_by_shell,
    .hit_by_star  = lakitu_hit_by_star,
};

void spawn_lakitu(Entity entities[MAX_ENTITIES], float x, float y, int extra) {
    (void)extra;
    Entity *e = entity_alloc(entities);
    if (!e) return;

    e->type = ENT_LAKITU;
    e->vtab = &lakitu_vtab;
    e->x = x;
    e->y = y;
    e->w = ENEMY_W;
    e->h = ENEMY_H;
    e->active = true;

    e->stompable = true;
    e->damages_mario = true;
    e->shell_killable = true;
    e->star_killable = true;
    e->destructible = true;
    e->self_moving = true;
}

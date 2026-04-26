#include "blooper.h"
#include "../game.h"
#include "../sounds.h"

// state_val: 0 = rising, 1 = pausing/sinking
// Blooper swims in pulses: rises diagonally toward Mario, then pauses and sinks.

static void blooper_update(Entity *self, Game *game) {
    float dt = GetFrameTime();
    Entity *mario = &game->entities[game->mario];

    self->state_timer += dt;

    if (self->state_val == 0) {
        // Rising phase: move up and toward Mario
        self->vy = -BLOOPER_RISE_SPEED;
        float dx = mario->x - self->x;
        if (dx > 0)
            self->vx = BLOOPER_CHASE_SPEED;
        else
            self->vx = -BLOOPER_CHASE_SPEED;

        self->x += self->vx * dt;
        self->y += self->vy * dt;

        if (self->y < TILE_SIZE) self->y = (float)TILE_SIZE;

        if (self->state_timer >= BLOOPER_RISE_TIME) {
            self->state_val = 1;
            self->state_timer = 0;
        }
    } else {
        // Sinking phase: drift down
        self->vx = 0;
        self->vy = BLOOPER_SINK_SPEED;
        self->y += self->vy * dt;

        float max_y = (float)(game->level.height * TILE_SIZE - TILE_SIZE * 2);
        if (self->y > max_y) self->y = max_y;

        if (self->state_timer >= BLOOPER_PAUSE_TIME) {
            self->state_val = 0;
            self->state_timer = 0;
        }
    }

    self->anim_timer += dt;

    if (self->x < game->camera_x - WINDOW_WIDTH || self->x > game->camera_x + WINDOW_WIDTH * 2)
        entity_deactivate(self);
}

static void blooper_draw(Entity *self, float camera_x) {
    float dx = self->x - camera_x;
    float dy = self->y;

    // Body (squid shape)
    DrawRectangle((int)(dx + 8), (int)dy, (int)(self->w - 16), (int)(self->h - 8), COLOR_BLOOPER);
    // Dome top
    DrawCircle((int)(dx + self->w / 2), (int)(dy + 8), self->w / 2 - 4, COLOR_BLOOPER);
    // Eyes
    DrawCircle((int)(dx + self->w * 0.35f), (int)(dy + 16), 5, BLACK);
    DrawCircle((int)(dx + self->w * 0.65f), (int)(dy + 16), 5, BLACK);
    // Tentacles
    float wiggle = sinf(self->anim_timer * 8.0f) * 4.0f;
    DrawRectangle((int)(dx + 10 + wiggle), (int)(dy + self->h - 12), 6, 12, COLOR_BLOOPER);
    DrawRectangle((int)(dx + self->w - 16 - wiggle), (int)(dy + self->h - 12), 6, 12, COLOR_BLOOPER);
    DrawRectangle((int)(dx + self->w / 2 - 3), (int)(dy + self->h - 10), 6, 10, COLOR_BLOOPER);
}

static void blooper_hit_by_fire(Entity *self, Game *game) {
    game->score += SCORE_FIREBALL_KILL;
    entity_deactivate(self);
}

static void blooper_hit_by_star(Entity *self, Game *game) {
    game->score += SCORE_GOOMBA_STOMP;
    entity_deactivate(self);
}

static const EntityVtab blooper_vtab = {
    .update      = blooper_update,
    .draw        = blooper_draw,
    .hit_by_fire = blooper_hit_by_fire,
    .hit_by_star = blooper_hit_by_star,
};

void spawn_blooper(Entity entities[MAX_ENTITIES], float x, float y, int extra) {
    (void)extra;
    Entity *e = entity_alloc(entities);
    if (!e) return;

    e->type = ENT_BLOOPER;
    e->vtab = &blooper_vtab;
    e->x = x;
    e->y = y;
    e->w = ENEMY_W;
    e->h = ENEMY_H;
    e->active = true;

    e->damages_mario = true;
    e->stompable = false;
    e->fire_immune = false;
    e->shell_killable = false;
    e->star_killable = true;
    e->destructible = true;
    e->self_moving = true;
}

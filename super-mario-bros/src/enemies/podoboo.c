#include "podoboo.h"
#include "../game.h"

static void podoboo_update(Entity *self, Game *game) {
    (void)game;
    float dt = GetFrameTime();

    // state_val: 0 = waiting in lava, 1 = jumping
    // anim_timer stores the base Y (lava surface)
    if (self->state_val == 0) {
        self->state_timer += dt;
        self->y = self->anim_timer;
        if (self->state_timer >= PODOBOO_INTERVAL) {
            self->state_val = 1;
            self->vy = PODOBOO_JUMP_VEL;
            self->state_timer = 0;
        }
    } else {
        self->vy += GRAVITY * dt;
        self->y += self->vy * dt;
        if (self->y >= self->anim_timer) {
            self->y = self->anim_timer;
            self->vy = 0;
            self->state_val = 0;
            self->state_timer = 0;
        }
    }
}

static void podoboo_draw(Entity *self, float camera_x) {
    float dx = self->x - camera_x;
    float dy = self->y;
    if (dy > self->anim_timer - 4) return;

    DrawCircle((int)(dx + self->w / 2), (int)(dy + self->h / 2),
               (float)(self->w / 2), COLOR_PODOBOO);
    // Flame trail
    float trail_h = fabsf(self->vy) * 0.02f;
    if (trail_h > 20) trail_h = 20;
    if (self->vy < 0) {
        DrawRectangle((int)(dx + 8), (int)(dy + self->h), (int)(self->w - 16),
                      (int)trail_h, (Color){255, 200, 0, 180});
    } else if (self->vy > 0) {
        DrawRectangle((int)(dx + 8), (int)(dy - trail_h), (int)(self->w - 16),
                      (int)trail_h, (Color){255, 200, 0, 180});
    }
}

static const EntityVtab podoboo_vtab = {
    .update = podoboo_update,
    .draw   = podoboo_draw,
};

void spawn_podoboo(Entity entities[MAX_ENTITIES], float x, float y, int extra) {
    (void)extra;
    Entity *e = entity_alloc(entities);
    if (!e) return;

    e->type = ENT_PODOBOO;
    e->vtab = &podoboo_vtab;
    e->x = x;
    e->y = y;
    e->w = PODOBOO_SIZE;
    e->h = PODOBOO_SIZE;
    e->active = true;
    e->anim_timer = y;
    e->state_val = 0;
    e->state_timer = 0;

    e->stompable = false;
    e->damages_mario = true;
    e->fire_immune = true;
    e->shell_killable = false;
    e->star_killable = false;
    e->destructible = false;
    e->self_moving = true;
}

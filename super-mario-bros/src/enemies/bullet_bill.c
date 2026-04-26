#include "bullet_bill.h"
#include "../game.h"
#include "../sounds.h"

static void bullet_update(Entity *self, Game *game) {
    float dt = GetFrameTime();
    self->x += self->vx * dt;
    self->anim_timer += dt;

    if (self->x < game->camera_x - TILE_SIZE * 2 ||
        self->x > game->camera_x + WINDOW_WIDTH + TILE_SIZE * 2)
        entity_deactivate(self);
}

static void bullet_draw(Entity *self, float camera_x) {
    float dx = self->x - camera_x;
    float dy = self->y;
    DrawRectangle((int)(dx + 4), (int)(dy + 8), (int)(self->w - 8), (int)(self->h - 16), BLACK);
    DrawEllipse((int)(dx + (self->facing == DIR_LEFT ? 8 : self->w - 8)),
                (int)(dy + self->h / 2), 12, (int)(self->h / 2 - 4), BLACK);
    DrawCircle((int)(dx + (self->facing == DIR_LEFT ? self->w - 12 : 12)),
               (int)(dy + self->h * 0.4f), 3, WHITE);
}

static void bullet_stomped(Entity *self, Entity *mario, Game *game) {
    (void)mario;
    game->score += SCORE_GOOMBA_STOMP;
    sound_play(SND_STOMP);
    entity_deactivate(self);
}

static void bullet_hit_by_shell(Entity *self, Game *game) {
    game->score += SCORE_SHELL_KILL;
    self->vy = -400.0f;
    self->damages_mario = false;
    self->stompable = false;
    self->dead_falling = true;
}

static void bullet_hit_by_star(Entity *self, Game *game) {
    game->score += SCORE_GOOMBA_STOMP;
    entity_deactivate(self);
}

static const EntityVtab bullet_vtab = {
    .update       = bullet_update,
    .draw         = bullet_draw,
    .stomped      = bullet_stomped,
    .hit_by_shell = bullet_hit_by_shell,
    .hit_by_star  = bullet_hit_by_star,
};

void spawn_bullet_bill(Entity entities[MAX_ENTITIES], float x, float y, int extra) {
    Entity *e = entity_alloc(entities);
    if (!e) return;

    e->type = ENT_BULLET_BILL;
    e->vtab = &bullet_vtab;
    e->x = x;
    e->y = y;
    e->w = ENEMY_W;
    e->h = ENEMY_H;
    e->vx = (extra <= 0) ? -BULLET_BILL_SPEED : BULLET_BILL_SPEED;
    e->facing = (e->vx < 0) ? DIR_LEFT : DIR_RIGHT;
    e->active = true;

    e->stompable = true;
    e->damages_mario = true;
    e->fire_immune = true;
    e->shell_killable = true;
    e->star_killable = true;
    e->destructible = true;
    e->self_moving = true;
}

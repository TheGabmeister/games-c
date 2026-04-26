#include "paratroopa.h"
#include "koopa.h"
#include "../game.h"
#include "../sounds.h"

static void paratroopa_update(Entity *self, Game *game) {
    float dt = GetFrameTime();

    self->vy += GRAVITY * dt;
    if (self->vy > MAX_FALL_SPEED) self->vy = MAX_FALL_SPEED;

    if (self->on_ground) {
        self->vy = PARATROOPA_BOUNCE_VEL;
    }

    self->anim_timer += dt;

    if (self->x < game->camera_x - WINDOW_WIDTH || self->x > game->camera_x + WINDOW_WIDTH * 2)
        entity_deactivate(self);
}

static void paratroopa_draw(Entity *self, float camera_x) {
    float dx = self->x - camera_x;
    float dy = self->y;
    DrawRectangle((int)dx, (int)dy, (int)self->w, (int)self->h, COLOR_PARATROOPA);
    // Head
    DrawCircle((int)(dx + self->w / 2), (int)(dy + 8), 12, COLOR_PARATROOPA);
    // Eyes
    float eye_x = (self->facing == DIR_LEFT) ? dx + self->w * 0.3f : dx + self->w * 0.7f;
    DrawCircle((int)eye_x, (int)(dy + 8), 4, WHITE);
    DrawCircle((int)eye_x, (int)(dy + 8), 2, BLACK);
    // Wings
    float flap = sinf(self->anim_timer * 10.0f) * 6.0f;
    DrawTriangle(
        (Vector2){dx + self->w * 0.2f, dy + 12 + flap},
        (Vector2){dx - 12, dy + 4 + flap},
        (Vector2){dx + self->w * 0.2f, dy + 28 + flap},
        WHITE);
    DrawTriangle(
        (Vector2){dx + self->w * 0.8f, dy + 12 + flap},
        (Vector2){dx + self->w + 12, dy + 4 + flap},
        (Vector2){dx + self->w * 0.8f, dy + 28 + flap},
        WHITE);
}

static void paratroopa_stomped(Entity *self, Entity *mario, Game *game) {
    (void)mario;
    game->score += SCORE_KOOPA_STOMP;
    sound_play(SND_STOMP);
    // Lose wings, become regular green koopa
    spawn_koopa(game->entities, self->x, self->y + (KOOPA_HEIGHT - TILE_SIZE), 0);
    entity_deactivate(self);
}

static void paratroopa_hit_by_fire(Entity *self, Game *game) {
    game->score += SCORE_FIREBALL_KILL;
    self->vy = -400.0f;
    self->damages_mario = false;
    self->stompable = false;
    self->state_val = 2;
    self->dead_falling = true;
}

static void paratroopa_hit_by_shell(Entity *self, Game *game) {
    game->score += SCORE_SHELL_KILL;
    self->vy = -400.0f;
    self->damages_mario = false;
    self->stompable = false;
    self->state_val = 2;
    self->dead_falling = true;
}

static void paratroopa_hit_by_star(Entity *self, Game *game) {
    game->score += SCORE_KOOPA_STOMP;
    self->vy = -400.0f;
    self->damages_mario = false;
    self->stompable = false;
    self->state_val = 2;
    self->dead_falling = true;
}

static void paratroopa_bumped(Entity *self, Game *game) {
    game->score += SCORE_KOOPA_STOMP;
    self->vy = -400.0f;
    self->vx = 0;
    self->damages_mario = false;
    self->stompable = false;
    self->state_val = 2;
    self->dead_falling = true;
}

static const EntityVtab paratroopa_vtab = {
    .update       = paratroopa_update,
    .draw         = paratroopa_draw,
    .stomped      = paratroopa_stomped,
    .hit_by_fire  = paratroopa_hit_by_fire,
    .hit_by_shell = paratroopa_hit_by_shell,
    .hit_by_star  = paratroopa_hit_by_star,
    .bumped       = paratroopa_bumped,
};

void spawn_paratroopa(Entity entities[MAX_ENTITIES], float x, float y, int extra) {
    (void)extra;
    Entity *e = entity_alloc(entities);
    if (!e) return;

    e->type = ENT_PARATROOPA;
    e->vtab = &paratroopa_vtab;
    e->x = x;
    e->y = y - (KOOPA_HEIGHT - TILE_SIZE);
    e->w = ENEMY_W;
    e->h = KOOPA_HEIGHT;
    e->vx = -PARATROOPA_SPEED;
    e->facing = DIR_LEFT;
    e->active = true;

    e->stompable = true;
    e->damages_mario = true;
    e->shell_killable = true;
    e->star_killable = true;
    e->destructible = true;
}

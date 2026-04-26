#include "cheep_cheep.h"
#include "../game.h"
#include "../sounds.h"

// state_val: 0 = gray/slow, 1 = red/fast, 2 = leaping
// anim_timer: used for sine wave and animation

// --- Swimming variant (underwater) ---

static void cheep_swim_update(Entity *self, Game *game) {
    float dt = GetFrameTime();
    self->anim_timer += dt;

    // Sine-wave vertical oscillation around base y (stored in state_timer at spawn)
    float base_y = self->state_timer;
    self->y = base_y + sinf(self->anim_timer * CHEEP_WAVE_FREQ * 2.0f * PI) * CHEEP_WAVE_AMP;

    self->x += self->vx * dt;

    if (self->x < game->camera_x - WINDOW_WIDTH || self->x > game->camera_x + WINDOW_WIDTH * 2)
        entity_deactivate(self);
}

// --- Leaping variant (bridge level) ---

static void cheep_leap_update(Entity *self, Game *game) {
    float dt = GetFrameTime();
    self->anim_timer += dt;

    self->vy += GRAVITY * dt;
    if (self->vy > MAX_FALL_SPEED) self->vy = MAX_FALL_SPEED;

    self->x += self->vx * dt;
    self->y += self->vy * dt;

    // Despawn if fallen below level
    if (self->y > game->level.height * TILE_SIZE + 200)
        entity_deactivate(self);

    if (self->x < game->camera_x - WINDOW_WIDTH || self->x > game->camera_x + WINDOW_WIDTH * 2)
        entity_deactivate(self);
}

// --- Shared drawing ---

static void cheep_draw(Entity *self, float camera_x) {
    float dx = self->x - camera_x;
    float dy = self->y;
    Color body_color = (self->state_val == 1) ? COLOR_CHEEP_RED : COLOR_CHEEP_GRAY;
    if (self->state_val == 2) body_color = COLOR_CHEEP_RED;

    // Body
    DrawRectangle((int)(dx + 4), (int)(dy + 4), (int)(self->w - 8), (int)(self->h - 8), body_color);
    // Tail fin
    float tail_x = (self->vx < 0) ? dx + self->w - 8 : dx;
    DrawTriangle(
        (Vector2){tail_x + 4, dy + self->h * 0.3f},
        (Vector2){tail_x + 4, dy + self->h * 0.7f},
        (Vector2){(self->vx < 0) ? tail_x + 14 : tail_x - 10, dy + self->h * 0.5f},
        body_color);
    // Eye
    float eye_x = (self->vx < 0) ? dx + self->w * 0.3f : dx + self->w * 0.7f;
    DrawCircle((int)eye_x, (int)(dy + self->h * 0.4f), 4, WHITE);
    DrawCircle((int)eye_x, (int)(dy + self->h * 0.4f), 2, BLACK);
    // Dorsal fin
    float fin_wobble = sinf(self->anim_timer * 8.0f) * 3.0f;
    DrawRectangle((int)(dx + self->w * 0.4f), (int)(dy + fin_wobble), (int)(self->w * 0.2f), 8, body_color);
}

// --- Stomp (leaping only) ---

static void cheep_stomped(Entity *self, Entity *mario, Game *game) {
    (void)mario;
    game->score += SCORE_GOOMBA_STOMP;
    sound_play(SND_STOMP);
    entity_deactivate(self);
}

static void cheep_hit_by_fire(Entity *self, Game *game) {
    game->score += SCORE_FIREBALL_KILL;
    entity_deactivate(self);
}

static void cheep_hit_by_star(Entity *self, Game *game) {
    game->score += SCORE_GOOMBA_STOMP;
    entity_deactivate(self);
}

// --- Vtables ---

static const EntityVtab cheep_swim_vtab = {
    .update      = cheep_swim_update,
    .draw        = cheep_draw,
    .hit_by_fire = cheep_hit_by_fire,
    .hit_by_star = cheep_hit_by_star,
};

static const EntityVtab cheep_leap_vtab = {
    .update      = cheep_leap_update,
    .draw        = cheep_draw,
    .stomped     = cheep_stomped,
    .hit_by_fire = cheep_hit_by_fire,
    .hit_by_star = cheep_hit_by_star,
};

void spawn_cheep_cheep(Entity entities[MAX_ENTITIES], float x, float y, int extra) {
    int variant = extra;
    Entity *e = entity_alloc(entities);
    if (!e) return;

    e->type = ENT_CHEEP_CHEEP;
    e->x = x;
    e->y = y;
    e->w = ENEMY_W;
    e->h = ENEMY_H;
    e->active = true;
    e->state_val = variant;
    e->damages_mario = true;
    e->star_killable = true;
    e->destructible = true;

    if (variant == 2) {
        // Leaping: jump up from below in a parabolic arc
        e->vtab = &cheep_leap_vtab;
        e->vy = CHEEP_LEAP_VEL;
        // Randomly go left or right
        e->vx = (GetRandomValue(0, 1) == 0) ? -CHEEP_LEAP_SPEED_X : CHEEP_LEAP_SPEED_X;
        e->facing = (e->vx < 0) ? DIR_LEFT : DIR_RIGHT;
        e->stompable = true;
    } else {
        // Swimming
        e->vtab = &cheep_swim_vtab;
        float speed = (variant == 1) ? CHEEP_FAST_SPEED : CHEEP_SLOW_SPEED;
        e->vx = -speed;
        e->facing = DIR_LEFT;
        e->stompable = false;
        e->self_moving = true;
        e->state_timer = y; // base Y for sine wave
    }
}

#include "koopa.h"
#include "../game.h"
#include "../sounds.h"

// --- Shell ---

static void shell_update(Entity *self, Game *game) {
    float dt = GetFrameTime();
    self->vy += GRAVITY * dt;
    if (self->vy > MAX_FALL_SPEED) self->vy = MAX_FALL_SPEED;

    // Stationary shell: can be kicked
    // Moving shell: damages mario and kills enemies
    if (fabsf(self->vx) < 1.0f) {
        self->damages_mario = false;
    } else {
        self->damages_mario = true;
    }

    // Despawn if far offscreen
    if (self->x < game->camera_x - WINDOW_WIDTH || self->x > game->camera_x + WINDOW_WIDTH * 2)
        entity_deactivate(self);
}

static void shell_draw(Entity *self, float camera_x) {
    float dx = self->x - camera_x;
    float dy = self->y;
    DrawRectangle((int)dx, (int)dy, (int)self->w, (int)self->h, COLOR_SHELL);
    DrawRectangleLines((int)dx, (int)dy, (int)self->w, (int)self->h, DARKGREEN);
}

static void shell_stomped(Entity *self, Entity *mario, Game *game) {
    (void)game;
    if (fabsf(self->vx) > 1.0f) {
        // Stop the shell
        self->vx = 0;
        sound_play(SND_STOMP);
    } else {
        // Kick the shell
        float kick_dir = (mario->x + mario->w / 2 < self->x + self->w / 2) ? 1.0f : -1.0f;
        self->vx = SHELL_SPEED * kick_dir;
        self->facing = (kick_dir > 0) ? DIR_RIGHT : DIR_LEFT;
        sound_play(SND_STOMP);
    }
}

static void shell_touch(Entity *self, Entity *other, Game *game) {
    (void)game;
    if (other->type == ENT_MARIO) {
        if (fabsf(self->vx) < 1.0f) {
            // Mario kicks stationary shell by walking into it
            float kick_dir = (other->x + other->w / 2 < self->x + self->w / 2) ? 1.0f : -1.0f;
            self->vx = SHELL_SPEED * kick_dir;
            self->facing = (kick_dir > 0) ? DIR_RIGHT : DIR_LEFT;
            sound_play(SND_STOMP);
        }
    }
}

static void shell_hit_by_shell(Entity *self, Game *game) {
    (void)game;
    self->vy = -400.0f;
    self->damages_mario = false;
    self->stompable = false;
    self->state_val = 2;
    self->dead_falling = true;
}

static const EntityVtab shell_vtab = {
    .update       = shell_update,
    .draw         = shell_draw,
    .stomped      = shell_stomped,
    .touch        = shell_touch,
    .hit_by_shell = shell_hit_by_shell,
};

void spawn_shell(Entity entities[MAX_ENTITIES], float x, float y, float kick_vx) {
    Entity *e = entity_alloc(entities);
    if (!e) return;

    e->type = ENT_SHELL;
    e->vtab = &shell_vtab;
    e->x = x;
    e->y = y;
    e->w = SHELL_W;
    e->h = SHELL_H;
    e->vx = kick_vx;
    e->facing = (kick_vx >= 0) ? DIR_RIGHT : DIR_LEFT;
    e->active = true;

    e->stompable = true;
    e->damages_mario = (fabsf(kick_vx) > 1.0f);
    e->shell_killable = true;
    e->star_killable = true;
    e->destructible = true;
}

// --- Koopa Troopa ---

static void koopa_update(Entity *self, Game *game) {
    float dt = GetFrameTime();
    self->vy += GRAVITY * dt;
    if (self->vy > MAX_FALL_SPEED) self->vy = MAX_FALL_SPEED;

    // Red Koopa: turn at ledges
    if (self->state_val == 1 && self->on_ground) {
        float check_x = (self->vx < 0) ? self->x - 4 : self->x + self->w + 4;
        int tx = (int)(check_x / TILE_SIZE);
        int ty = (int)((self->y + self->h + 4) / TILE_SIZE);
        if (!tile_is_solid(level_get_tile(&game->level, tx, ty))) {
            self->vx = -self->vx;
            self->facing = (self->vx < 0) ? DIR_LEFT : DIR_RIGHT;
        }
    }

    self->anim_timer += dt;

    if (self->x < game->camera_x - WINDOW_WIDTH || self->x > game->camera_x + WINDOW_WIDTH * 2)
        entity_deactivate(self);
}

static void koopa_draw(Entity *self, float camera_x) {
    float dx = self->x - camera_x;
    float dy = self->y;
    DrawRectangle((int)dx, (int)dy, (int)self->w, (int)self->h, COLOR_KOOPA);
    // Head
    DrawCircle((int)(dx + self->w / 2), (int)(dy + 8), 12, COLOR_KOOPA);
    // Eyes
    float eye_x = (self->facing == DIR_LEFT) ? dx + self->w * 0.3f : dx + self->w * 0.7f;
    DrawCircle((int)eye_x, (int)(dy + 8), 4, WHITE);
    DrawCircle((int)eye_x, (int)(dy + 8), 2, BLACK);
}

static void koopa_stomped(Entity *self, Entity *mario, Game *game) {
    (void)mario;
    game->score += SCORE_KOOPA_STOMP;
    sound_play(SND_STOMP);
    // Transform into shell
    spawn_shell(game->entities, self->x, self->y + (KOOPA_HEIGHT - SHELL_H), 0);
    entity_deactivate(self);
}

static void koopa_hit_by_fire(Entity *self, Game *game) {
    game->score += SCORE_FIREBALL_KILL;
    self->vy = -400.0f;
    self->damages_mario = false;
    self->stompable = false;
    self->state_val = 2;
    self->dead_falling = true;
}

static void koopa_hit_by_shell(Entity *self, Game *game) {
    game->score += SCORE_SHELL_KILL;
    self->vy = -400.0f;
    self->damages_mario = false;
    self->stompable = false;
    self->state_val = 2;
    self->dead_falling = true;
}

static void koopa_hit_by_star(Entity *self, Game *game) {
    game->score += SCORE_KOOPA_STOMP;
    self->vy = -400.0f;
    self->damages_mario = false;
    self->stompable = false;
    self->state_val = 2;
    self->dead_falling = true;
}

static void koopa_bumped(Entity *self, Game *game) {
    game->score += SCORE_KOOPA_STOMP;
    self->vy = -400.0f;
    self->vx = 0;
    self->damages_mario = false;
    self->stompable = false;
    self->state_val = 2;
    self->dead_falling = true;
}

static void koopa_touch(Entity *self, Entity *other, Game *game) {
    (void)game;
    if (other->type != ENT_MARIO && other->type != ENT_FIREBALL) {
        self->vx = -self->vx;
        self->facing = (self->vx < 0) ? DIR_LEFT : DIR_RIGHT;
    }
}

static const EntityVtab koopa_vtab = {
    .update       = koopa_update,
    .draw         = koopa_draw,
    .touch        = koopa_touch,
    .stomped      = koopa_stomped,
    .hit_by_fire  = koopa_hit_by_fire,
    .hit_by_shell = koopa_hit_by_shell,
    .hit_by_star  = koopa_hit_by_star,
    .bumped       = koopa_bumped,
};

void spawn_koopa(Entity entities[MAX_ENTITIES], float x, float y, int extra) {
    bool red = (extra != 0);
    Entity *e = entity_alloc(entities);
    if (!e) return;

    e->type = ENT_KOOPA;
    e->vtab = &koopa_vtab;
    e->x = x;
    e->y = y - (KOOPA_HEIGHT - TILE_SIZE);
    e->w = ENEMY_W;
    e->h = KOOPA_HEIGHT;
    e->vx = -KOOPA_SPEED;
    e->facing = DIR_LEFT;
    e->active = true;
    e->state_val = red ? 1 : 0;

    e->stompable = true;
    e->damages_mario = true;
    e->shell_killable = true;
    e->star_killable = true;
    e->destructible = true;
}

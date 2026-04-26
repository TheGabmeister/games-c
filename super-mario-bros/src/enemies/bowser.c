#include "bowser.h"
#include "hammer_bro.h"
#include "../game.h"
#include "../sounds.h"

// --- Bowser's Fireball ---

static void bowser_fire_update(Entity *self, Game *game) {
    float dt = GetFrameTime();
    self->x += self->vx * dt;
    self->anim_timer += dt;

    if (self->x < game->camera_x - TILE_SIZE || self->x > game->camera_x + WINDOW_WIDTH + TILE_SIZE)
        entity_deactivate(self);
}

static void bowser_fire_draw(Entity *self, float camera_x) {
    float dx = self->x - camera_x;
    float dy = self->y;
    float pulse = sinf(self->anim_timer * 12.0f) * 0.2f + 0.8f;
    Color c = {(unsigned char)(255 * pulse), (unsigned char)(80 * pulse), 0, 255};
    DrawRectangle((int)dx, (int)dy, (int)self->w, (int)self->h, c);
}

static const EntityVtab bowser_fire_vtab = {
    .update = bowser_fire_update,
    .draw   = bowser_fire_draw,
};

static void spawn_bowser_fireball(Entity entities[MAX_ENTITIES], float x, float y, Direction dir) {
    Entity *e = entity_alloc(entities);
    if (!e) return;

    e->type = ENT_BOWSER_FIRE;
    e->vtab = &bowser_fire_vtab;
    e->x = x;
    e->y = y;
    e->w = BOWSER_FIRE_W;
    e->h = BOWSER_FIRE_H;
    e->vx = (dir == DIR_LEFT) ? -BOWSER_FIREBALL_SPEED : BOWSER_FIREBALL_SPEED;
    e->active = true;
    e->facing = dir;

    e->stompable = false;
    e->damages_mario = true;
    e->fire_immune = true;
    e->shell_killable = false;
    e->star_killable = false;
    e->destructible = false;
    e->self_moving = true;
}

// --- Bowser ---

static void bowser_update(Entity *self, Game *game) {
    float dt = GetFrameTime();
    Entity *mario = &game->entities[game->mario];

    // Gravity
    self->vy += GRAVITY * dt;
    if (self->vy > MAX_FALL_SPEED) self->vy = MAX_FALL_SPEED;

    // Movement — walk back and forth
    self->state_timer += dt;
    float walk_cycle = sinf(self->state_timer * 0.8f);
    self->vx = walk_cycle * BOWSER_SPEED;
    self->facing = (mario->x < self->x) ? DIR_LEFT : DIR_RIGHT;

    // Tile collision
    self->x += self->vx * dt;
    self->y += self->vy * dt;

    // Simple ground check — snap to bridge/ground
    int bot_ty = (int)((self->y + self->h) / TILE_SIZE);
    int left_tx = (int)(self->x / TILE_SIZE);
    int right_tx = (int)((self->x + self->w - 1) / TILE_SIZE);
    self->on_ground = false;
    for (int tx = left_tx; tx <= right_tx; tx++) {
        int tile = level_get_tile(&game->level, tx, bot_ty);
        if (tile_is_solid(tile) || tile == TILE_BRIDGE) {
            self->y = (float)(bot_ty * TILE_SIZE) - self->h;
            self->vy = 0;
            self->on_ground = true;
            break;
        }
    }

    // Jump periodically
    // anim_timer used for jump cooldown
    self->anim_timer += dt;
    if (self->on_ground && self->anim_timer >= BOWSER_JUMP_INTERVAL) {
        self->vy = BOWSER_JUMP_VEL;
        self->on_ground = false;
        self->anim_timer = 0;
    }

    // Fire breath
    float prev = self->state_timer - dt;
    if (prev < 0) prev = 0;
    if ((int)(self->state_timer / BOWSER_FIRE_INTERVAL) != (int)(prev / BOWSER_FIRE_INTERVAL)) {
        float fx = (self->facing == DIR_LEFT) ? self->x - BOWSER_FIRE_W : self->x + self->w;
        float fy = self->y + BOWSER_HEIGHT * 0.3f;
        spawn_bowser_fireball(game->entities, fx, fy, self->facing);
    }

    // Hammer throwing (world 6+)
    if (game->world >= 6) {
        if ((int)(self->state_timer / BOWSER_HAMMER_INTERVAL) != (int)(prev / BOWSER_HAMMER_INTERVAL)) {
            float hx = self->x + self->w / 2;
            float hy = self->y;
            spawn_hammer_projectile(game->entities, hx, hy, self->facing);
        }
    }
}

static void bowser_draw(Entity *self, float camera_x) {
    float dx = self->x - camera_x;
    float dy = self->y;

    // Body
    DrawRectangle((int)dx, (int)dy, (int)self->w, (int)self->h, COLOR_BOWSER);
    // Shell
    DrawRectangle((int)(dx + 8), (int)(dy + 8), (int)(self->w - 16), (int)(self->h - 24), (Color){60, 100, 0, 255});
    // Horns
    float head_x = (self->facing == DIR_LEFT) ? dx : dx + self->w - 24;
    DrawTriangle(
        (Vector2){head_x + 4, dy + 8},
        (Vector2){head_x, dy - 8},
        (Vector2){head_x + 12, dy + 4},
        (Color){200, 180, 100, 255});
    DrawTriangle(
        (Vector2){head_x + 16, dy + 8},
        (Vector2){head_x + 20, dy - 8},
        (Vector2){head_x + 8, dy + 4},
        (Color){200, 180, 100, 255});
    // Eyes
    float eye_x = (self->facing == DIR_LEFT) ? dx + 12 : dx + self->w - 20;
    DrawCircle((int)eye_x, (int)(dy + 24), 6, WHITE);
    DrawCircle((int)eye_x, (int)(dy + 24), 3, RED);
    // Mouth/fire indicator
    float mouth_x = (self->facing == DIR_LEFT) ? dx : dx + self->w - 12;
    DrawRectangle((int)mouth_x, (int)(dy + 32), 12, 8, (Color){200, 0, 0, 255});
}

static void bowser_hit_by_fire(Entity *self, Game *game) {
    self->state_val--;
    if (self->state_val <= 0) {
        game->score += 5000;
        self->vy = -400.0f;
        self->damages_mario = false;
        self->stompable = false;
        if (game->world >= 8) {
            self->state_val = 2;
            self->dead_falling = true;
        } else {
            self->type = ENT_GOOMBA;
            self->state_val = 2;
            self->dead_falling = true;
        }
    }
}

static const EntityVtab bowser_vtab = {
    .update      = bowser_update,
    .draw        = bowser_draw,
    .hit_by_fire = bowser_hit_by_fire,
};

void spawn_bowser(Entity entities[MAX_ENTITIES], float x, float y, int extra) {
    (void)extra;
    Entity *e = entity_alloc(entities);
    if (!e) return;

    e->type = ENT_BOWSER;
    e->vtab = &bowser_vtab;
    e->x = x;
    e->y = y - (BOWSER_HEIGHT - TILE_SIZE);
    e->w = BOWSER_W;
    e->h = BOWSER_HEIGHT;
    e->active = true;
    e->facing = DIR_LEFT;
    e->state_val = BOWSER_HP;
    e->state_timer = 0;
    e->anim_timer = 0;

    e->stompable = false;
    e->damages_mario = true;
    e->fire_immune = false;
    e->shell_killable = false;
    e->star_killable = false;
    e->destructible = true;
    e->self_moving = true;
}

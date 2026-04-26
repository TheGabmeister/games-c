#include "piranha.h"
#include "../game.h"
#include "../sounds.h"

static void piranha_update(Entity *self, Game *game) {
    float dt = GetFrameTime();
    Entity *mario = &game->entities[game->mario];

    // state_val: 0 = hidden, 1 = rising, 2 = paused at top, 3 = sinking
    // state_timer: tracks time in current state
    // anim_timer: stores the base Y (pipe top)

    float base_y = self->anim_timer;
    float dist_to_mario = fabsf((mario->x + mario->w / 2) - (self->x + self->w / 2));

    switch (self->state_val) {
        case 0: // Hidden inside pipe
            self->y = base_y + PIRANHA_HEIGHT;
            self->state_timer += dt;
            if (self->state_timer >= 1.0f && dist_to_mario > PIRANHA_HIDE_DIST) {
                self->state_val = 1;
                self->state_timer = 0;
            }
            break;
        case 1: // Rising
            self->y -= PIRANHA_RISE_SPEED * dt;
            if (self->y <= base_y) {
                self->y = base_y;
                self->state_val = 2;
                self->state_timer = 0;
            }
            break;
        case 2: // Paused at top
            self->state_timer += dt;
            if (self->state_timer >= PIRANHA_PAUSE_TIME) {
                self->state_val = 3;
                self->state_timer = 0;
            }
            break;
        case 3: // Sinking
            self->y += PIRANHA_RISE_SPEED * dt;
            if (self->y >= base_y + PIRANHA_HEIGHT) {
                self->y = base_y + PIRANHA_HEIGHT;
                self->state_val = 0;
                self->state_timer = 0;
            }
            break;
    }
}

static void piranha_draw(Entity *self, float camera_x) {
    float dx = self->x - camera_x;
    float dy = self->y;

    float base_y = self->anim_timer;
    float visible_h = (base_y + PIRANHA_HEIGHT) - self->y;
    if (visible_h <= 0) return;
    if (visible_h > self->h) visible_h = self->h;

    // Head (bulb)
    DrawRectangle((int)(dx + 4), (int)dy, (int)(self->w - 8), (int)(visible_h * 0.5f), COLOR_PIRANHA);
    // White dots on head
    DrawCircle((int)(dx + 12), (int)(dy + 8), 4, WHITE);
    DrawCircle((int)(dx + self->w - 12), (int)(dy + 8), 4, WHITE);
    // Stem
    if (visible_h > self->h * 0.5f) {
        float stem_y = dy + self->h * 0.5f;
        float stem_h = visible_h - self->h * 0.5f;
        DrawRectangle((int)(dx + 12), (int)stem_y, (int)(self->w - 24), (int)stem_h, (Color){0, 120, 0, 255});
    }
}

static void piranha_hit_by_fire(Entity *self, Game *game) {
    game->score += SCORE_FIREBALL_KILL;
    entity_deactivate(self);
}

static void piranha_hit_by_shell(Entity *self, Game *game) {
    game->score += SCORE_SHELL_KILL;
    entity_deactivate(self);
}

static void piranha_hit_by_star(Entity *self, Game *game) {
    game->score += SCORE_GOOMBA_STOMP;
    entity_deactivate(self);
}

static const EntityVtab piranha_vtab = {
    .update       = piranha_update,
    .draw         = piranha_draw,
    .hit_by_fire  = piranha_hit_by_fire,
    .hit_by_shell = piranha_hit_by_shell,
    .hit_by_star  = piranha_hit_by_star,
};

void spawn_piranha(Entity entities[MAX_ENTITIES], float x, float y, int extra) {
    (void)extra;
    Entity *e = entity_alloc(entities);
    if (!e) return;

    e->type = ENT_PIRANHA;
    e->vtab = &piranha_vtab;
    // x,y is the pipe top-left tile position; center the piranha on the pipe
    e->x = x + (TILE_SIZE - PIRANHA_W) / 2;
    e->w = PIRANHA_W;
    e->h = PIRANHA_HEIGHT;
    e->active = true;

    // Store base Y (pipe top) in anim_timer
    e->anim_timer = y - PIRANHA_HEIGHT + TILE_SIZE * 0.25f;
    e->y = e->anim_timer + PIRANHA_HEIGHT; // start hidden
    e->state_val = 0;
    e->state_timer = 0;

    e->stompable = false;
    e->damages_mario = true;
    e->fire_immune = false;
    e->shell_killable = true;
    e->star_killable = true;
    e->destructible = true;
    e->self_moving = true;
}

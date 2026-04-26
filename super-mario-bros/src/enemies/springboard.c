#include "springboard.h"
#include "../game.h"
#include "../sounds.h"

// state_val: 0 = idle, 1 = compressed (visual feedback)

static void springboard_update(Entity *self, Game *game) {
    (void)game;
    float dt = GetFrameTime();
    self->vy += GRAVITY * dt;
    if (self->vy > MAX_FALL_SPEED) self->vy = MAX_FALL_SPEED;
    if (self->state_val == 1) {
        self->state_timer -= dt;
        if (self->state_timer <= 0) self->state_val = 0;
    }
}

static void springboard_draw(Entity *self, float camera_x) {
    float dx = self->x - camera_x;
    float dy = self->y;

    if (self->state_val == 1) {
        // Compressed
        DrawRectangle((int)dx, (int)(dy + self->h - 24), (int)self->w, 24, COLOR_SPRINGBOARD);
        DrawRectangle((int)(dx + 4), (int)(dy + self->h - 28), (int)(self->w - 8), 8, YELLOW);
    } else {
        // Normal
        DrawRectangle((int)dx, (int)dy, (int)self->w, (int)self->h, COLOR_SPRINGBOARD);
        // Spring coil lines
        DrawRectangle((int)(dx + 4), (int)(dy + 8), (int)(self->w - 8), 4, YELLOW);
        DrawRectangle((int)(dx + 4), (int)(dy + 20), (int)(self->w - 8), 4, YELLOW);
        DrawRectangle((int)(dx + 4), (int)(dy + 32), (int)(self->w - 8), 4, YELLOW);
        // Top platform
        DrawRectangle((int)dx, (int)dy, (int)self->w, 6, DARKGREEN);
    }
}

static void springboard_touch(Entity *self, Entity *other, Game *game) {
    (void)game;
    if (other->type != ENT_MARIO) return;
    if (other->vy <= 0) return;

    // Mario must be landing on top
    float mario_feet = other->y + other->h;
    if (mario_feet < self->y || mario_feet > self->y + self->h * 0.5f) return;

    bool jump_held = IsKeyDown(KEY_SPACE) || IsKeyDown(KEY_UP) || IsKeyDown(KEY_W) ||
                     (IsGamepadAvailable(0) && (IsGamepadButtonDown(0, GAMEPAD_BUTTON_RIGHT_FACE_DOWN) ||
                                                IsGamepadButtonDown(0, GAMEPAD_BUTTON_LEFT_FACE_UP)));

    other->vy = jump_held ? SPRINGBOARD_BOUNCE_STRONG : SPRINGBOARD_BOUNCE_VEL;
    other->on_ground = false;
    other->jumping = true;
    other->y = self->y - other->h;

    self->state_val = 1;
    self->state_timer = 0.2f;
    sound_play(SND_JUMP);
}

static const EntityVtab springboard_vtab = {
    .update = springboard_update,
    .draw   = springboard_draw,
    .touch  = springboard_touch,
};

void spawn_springboard(Entity entities[MAX_ENTITIES], float x, float y, int extra) {
    (void)extra;
    Entity *e = entity_alloc(entities);
    if (!e) return;

    e->type = ENT_SPRINGBOARD;
    e->vtab = &springboard_vtab;
    e->x = x;
    e->y = y;
    e->w = SPRINGBOARD_W;
    e->h = SPRINGBOARD_HEIGHT;
    e->active = true;
    e->damages_mario = false;
    e->stompable = false;
}

#include "firebar.h"
#include "../game.h"

static void firebar_update(Entity *self, Game *game) {
    (void)game;
    float dt = GetFrameTime();
    float dir = (self->state_val != 0) ? -1.0f : 1.0f;
    self->anim_timer += FIREBAR_ANGULAR_SPEED * dir * dt;
}

static void firebar_draw(Entity *self, float camera_x) {
    float cx = self->x + TILE_SIZE / 2 - camera_x;
    float cy = self->y + TILE_SIZE / 2;
    float angle = self->anim_timer;

    for (int i = 1; i <= FIREBAR_BALL_COUNT; i++) {
        float dist = (float)(i * FIREBAR_BALL_SPACING);
        float bx = cx + cosf(angle) * dist;
        float by = cy + sinf(angle) * dist;
        float pulse = sinf((float)GetTime() * 8.0f + i) * 0.2f + 0.8f;
        Color c = {(unsigned char)(255 * pulse), (unsigned char)(100 * pulse), 0, 255};
        DrawCircle((int)bx, (int)by, FIREBAR_BALL_RADIUS, c);
    }

    // Anchor block
    DrawRectangle((int)(cx - 8), (int)(cy - 8), 16, 16, GRAY);
}

static const EntityVtab firebar_vtab = {
    .update = firebar_update,
    .draw   = firebar_draw,
};

void spawn_firebar(Entity entities[MAX_ENTITIES], float x, float y, int clockwise) {
    Entity *e = entity_alloc(entities);
    if (!e) return;

    e->type = ENT_FIREBAR;
    e->vtab = &firebar_vtab;
    e->x = x;
    e->y = y;
    e->w = TILE_SIZE;
    e->h = TILE_SIZE;
    e->active = true;
    e->state_val = clockwise;
    e->anim_timer = 0;

    e->stompable = false;
    e->damages_mario = true;
    e->fire_immune = true;
    e->shell_killable = false;
    e->star_killable = false;
    e->destructible = false;
}

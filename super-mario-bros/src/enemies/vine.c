#include "vine.h"
#include "../game.h"
#include "../sounds.h"
#include "../level.h"

// Vine grows upward from a block. Mario can grab it by pressing Up while overlapping.
// Grabbing the vine triggers a pipe warp to the sky bonus area (uses existing warp system).
// state_val: current height in pixels (grows over time)
// anim_timer: base Y (block position)

static void vine_update(Entity *self, Game *game) {
    float dt = GetFrameTime();

    // Grow upward
    if (self->state_val < (int)VINE_MAX_HEIGHT) {
        self->state_val += (int)(VINE_GROW_SPEED * dt);
        if (self->state_val > (int)VINE_MAX_HEIGHT)
            self->state_val = (int)VINE_MAX_HEIGHT;
        self->y = self->anim_timer - self->state_val;
        self->h = (float)self->state_val;
    }
}

static void vine_draw(Entity *self, float camera_x) {
    float dx = self->x - camera_x + (TILE_SIZE - VINE_W) / 2;
    float dy = self->y;
    float height = self->h;

    // Main stem
    DrawRectangle((int)(dx + VINE_W / 2 - 3), (int)dy, 6, (int)height, COLOR_VINE);

    // Leaves alternating sides
    int leaf_count = (int)(height / 32);
    for (int i = 0; i < leaf_count; i++) {
        float ly = dy + height - i * 32 - 16;
        if (ly < dy) break;
        float lx = (i % 2 == 0) ? dx - 8 : dx + VINE_W;
        DrawEllipse((int)(lx + 6), (int)(ly + 4), 10, 6, COLOR_VINE);
    }
}

static void vine_touch(Entity *self, Entity *other, Game *game) {
    if (other->type != ENT_MARIO) return;

    // Mario grabs the vine by pressing Up
    bool up_pressed = IsKeyDown(KEY_UP) || IsKeyDown(KEY_W) ||
                      (IsGamepadAvailable(0) && IsGamepadButtonDown(0, GAMEPAD_BUTTON_LEFT_FACE_UP));
    if (!up_pressed) return;

    // Look for a vine warp destination in the level
    int vine_tx = (int)(self->x / TILE_SIZE);
    int vine_ty = (int)(self->anim_timer / TILE_SIZE);
    PipeWarp *w = level_get_warp(&game->level, vine_tx, vine_ty);
    if (w) {
        game->warp_world = w->dest_world;
        game->warp_sublevel = w->dest_sublevel;
        game->warp_tx = w->dest_tx;
        game->warp_ty = w->dest_ty;
        game->state = STATE_PIPE_TRANSITION;
        game->state_timer = 0;
        game->saved_power = other->power;
        sound_play(SND_PIPE);
    }
}

static const EntityVtab vine_vtab = {
    .update = vine_update,
    .draw   = vine_draw,
    .touch  = vine_touch,
};

void spawn_vine(Entity entities[MAX_ENTITIES], float x, float y) {
    Entity *e = entity_alloc(entities);
    if (!e) return;

    e->type = ENT_VINE;
    e->vtab = &vine_vtab;
    e->x = x;
    e->y = y;
    e->w = TILE_SIZE;
    e->h = 1;
    e->active = true;
    e->anim_timer = y;
    e->state_val = 0;

    e->damages_mario = false;
    e->stompable = false;
    e->self_moving = true;
}

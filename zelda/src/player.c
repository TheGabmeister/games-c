#include "player.h"
#include "input.h"
#include "textures.h"
#include "game_config.h"
#include "raymath.h"
#include <math.h>
#include <string.h>

const AnimDef player_idle_anims[DIR_COUNT] = {
    [DIR_S] = { 0,  1, 1, false },
    [DIR_N] = { 4,  1, 1, false },
    [DIR_E] = { 8,  1, 1, false },
    [DIR_W] = { 12, 1, 1, false },
};

const AnimDef player_walk_anims[DIR_COUNT] = {
    [DIR_S] = { 0,  4, PLAYER_ANIM_FRAMES, true },
    [DIR_N] = { 4,  4, PLAYER_ANIM_FRAMES, true },
    [DIR_E] = { 8,  4, PLAYER_ANIM_FRAMES, true },
    [DIR_W] = { 12, 4, PLAYER_ANIM_FRAMES, true },
};

void player_init(Player *player) {
    memset(player, 0, sizeof(*player));
    player->pos = (Vector2){ 7 * TILE_SIZE, PLAY_AREA_Y + 5 * TILE_SIZE };
    player->facing = DIR_S;
    player->state = PSTATE_IDLE;
    player->health = PLAYER_MAX_HEALTH;
    player->max_health = PLAYER_MAX_HEALTH;
    player->inventory.bomb_capacity = 8;
    anim_set(&player->anim, &player_idle_anims[DIR_S]);
}

static float snap_toward_grid(float pos, float base, float speed, float dt) {
    float rel = pos - base;
    float nearest = floorf(rel / TILE_SIZE + 0.5f) * TILE_SIZE;
    float diff = (base + nearest) - pos;
    if (fabsf(diff) < 0.5f) return pos;
    float step = speed * dt;
    if (fabsf(diff) <= step) return pos + diff;
    return pos + (diff > 0 ? step : -step);
}

void player_update(Player *player, const Screen *screen, float dt) {
    bool left  = input_left();
    bool right = input_right();
    bool up    = input_up();
    bool down  = input_down();

    float move_x = 0, move_y = 0;
    Direction new_facing = player->facing;

    if (left && !right)  { move_x = -1; new_facing = DIR_W; }
    if (right && !left)  { move_x =  1; new_facing = DIR_E; }
    if (up && !down)     { move_y = -1; new_facing = DIR_N; }
    if (down && !up)     { move_y =  1; new_facing = DIR_S; }

    // Four-directional only: if both axes have input, keep only the most
    // recent direction. Since we can't track press order with held-state
    // input, vertical wins ties (matches NES Zelda behavior).
    if (move_x != 0 && move_y != 0) {
        move_x = 0;
    }

    bool moving = (move_x != 0 || move_y != 0);

    if (moving) {
        player->facing = new_facing;
        float step = PLAYER_SPEED * dt;

        if (move_x != 0) {
            float new_x = player->pos.x + move_x * step;
            new_x = Clamp(new_x, 0, SCREEN_TILES_X * TILE_SIZE - TILE_SIZE);
            Rectangle test = { new_x, player->pos.y, TILE_SIZE, TILE_SIZE };
            if (!screen_tile_blocked(screen, test)) {
                player->pos.x = new_x;
            }
            // Grid assist: snap Y toward nearest tile row
            float snapped_y = snap_toward_grid(player->pos.y, PLAY_AREA_Y, PLAYER_SPEED, dt);
            snapped_y = Clamp(snapped_y, PLAY_AREA_Y, PLAY_AREA_Y + SCREEN_TILES_Y * TILE_SIZE - TILE_SIZE);
            Rectangle snap_test = { player->pos.x, snapped_y, TILE_SIZE, TILE_SIZE };
            if (!screen_tile_blocked(screen, snap_test)) {
                player->pos.y = snapped_y;
            }
        } else {
            float new_y = player->pos.y + move_y * step;
            new_y = Clamp(new_y, PLAY_AREA_Y, PLAY_AREA_Y + SCREEN_TILES_Y * TILE_SIZE - TILE_SIZE);
            Rectangle test = { player->pos.x, new_y, TILE_SIZE, TILE_SIZE };
            if (!screen_tile_blocked(screen, test)) {
                player->pos.y = new_y;
            }
            // Grid assist: snap X toward nearest tile column
            float snapped_x = snap_toward_grid(player->pos.x, 0, PLAYER_SPEED, dt);
            snapped_x = Clamp(snapped_x, 0, SCREEN_TILES_X * TILE_SIZE - TILE_SIZE);
            Rectangle snap_test = { snapped_x, player->pos.y, TILE_SIZE, TILE_SIZE };
            if (!screen_tile_blocked(screen, snap_test)) {
                player->pos.x = snapped_x;
            }
        }

        player->state = PSTATE_MOVING;
    } else {
        player->state = PSTATE_IDLE;
    }

    const AnimDef *desired = (player->state == PSTATE_MOVING)
        ? &player_walk_anims[player->facing]
        : &player_idle_anims[player->facing];

    if (player->anim.def != desired) {
        anim_set(&player->anim, desired);
    }
    anim_tick(&player->anim);
}

void player_draw(const Player *player) {
    int frame = anim_frame_index(&player->anim);
    if (IsTextureValid(textures[TEX_PLAYER])) {
        Rectangle src = texture_frame_rect(4, frame);
        DrawTextureRec(textures[TEX_PLAYER], src, player->pos, WHITE);
    } else {
        DrawRectangle((int)player->pos.x, (int)player->pos.y, TILE_SIZE, TILE_SIZE, GREEN);
    }
}

Rectangle player_hitbox(const Player *player) {
    return (Rectangle){ player->pos.x, player->pos.y, TILE_SIZE, TILE_SIZE };
}

#include "player.h"
#include "input.h"
#include "textures.h"
#include "game_config.h"
#include "raymath.h"
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

void player_update(Player *player, const Screen *screen, float dt) {
    Vector2 move = { 0 };
    Direction new_facing = player->facing;

    if (input_left())  { move.x -= 1.0f; new_facing = DIR_W; }
    if (input_right()) { move.x += 1.0f; new_facing = DIR_E; }
    if (input_up())    { move.y -= 1.0f; new_facing = DIR_N; }
    if (input_down())  { move.y += 1.0f; new_facing = DIR_S; }

    bool moving = (move.x != 0.0f || move.y != 0.0f);

    if (moving) {
        player->facing = new_facing;
        move = Vector2Normalize(move);
        move.x *= PLAYER_SPEED * dt;
        move.y *= PLAYER_SPEED * dt;

        float new_x = player->pos.x + move.x;
        new_x = Clamp(new_x, 0, SCREEN_TILES_X * TILE_SIZE - TILE_SIZE);
        Rectangle test = { new_x, player->pos.y, TILE_SIZE, TILE_SIZE };
        if (!screen_tile_blocked(screen, test)) {
            player->pos.x = new_x;
        }

        float new_y = player->pos.y + move.y;
        new_y = Clamp(new_y, PLAY_AREA_Y, PLAY_AREA_Y + SCREEN_TILES_Y * TILE_SIZE - TILE_SIZE);
        test = (Rectangle){ player->pos.x, new_y, TILE_SIZE, TILE_SIZE };
        if (!screen_tile_blocked(screen, test)) {
            player->pos.y = new_y;
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

#include "player.h"
#include "input.h"
#include "textures.h"
#include "projectile.h"
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

static const AnimDef player_attack_anims[DIR_COUNT] = {
    [DIR_S] = { 16, 2, 4, false },
    [DIR_N] = { 18, 2, 4, false },
    [DIR_E] = { 20, 2, 4, false },
    [DIR_W] = { 22, 2, 4, false },
};

void player_init(Player *player) {
    memset(player, 0, sizeof(*player));
    player->pos = (Vector2){ 7 * TILE_SIZE, PLAY_AREA_Y + 5 * TILE_SIZE };
    player->facing = DIR_S;
    player->state = PSTATE_IDLE;
    player->health = PLAYER_MAX_HEALTH;
    player->max_health = PLAYER_MAX_HEALTH;
    player->inventory.sword_tier = 1;
    player->inventory.shield_tier = 1;
    player->inventory.bomb_capacity = 8;
    player->inventory.bombs = 8;
    player->inventory.arrow_capacity = 30;
    player->inventory.arrows = 30;
    player->inventory.rupees = 50;
    player->inventory.equipped = ITEM_BOW;
    player->inventory.items = (1 << ITEM_BOOMERANG) | (1 << ITEM_BOW) | (1 << ITEM_CANDLE);
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

static void try_use_item(Player *player, Projectile *projectiles, int *projectile_count) {
    switch (player->inventory.equipped) {
        case ITEM_BOW:
            if (player->inventory.arrows <= 0) return;
            player->inventory.arrows--;
            projectile_spawn(projectiles, projectile_count,
                             PROJ_ARROW, OWNER_PLAYER, player->pos, player->facing);
            break;
        case ITEM_BOOMERANG:
            for (int i = 0; i < *projectile_count; i++) {
                if (projectiles[i].active && projectiles[i].type == PROJ_BOOMERANG &&
                    projectiles[i].owner == OWNER_PLAYER) return;
            }
            projectile_spawn(projectiles, projectile_count,
                             PROJ_BOOMERANG, OWNER_PLAYER, player->pos, player->facing);
            break;
        case ITEM_BOMB:
            if (player->inventory.bombs <= 0) return;
            for (int i = 0; i < *projectile_count; i++) {
                if (projectiles[i].active && projectiles[i].type == PROJ_BOMB &&
                    projectiles[i].owner == OWNER_PLAYER) return;
            }
            player->inventory.bombs--;
            projectile_spawn(projectiles, projectile_count,
                             PROJ_BOMB, OWNER_PLAYER, player->pos, player->facing);
            break;
        case ITEM_CANDLE:
            break;
        default:
            return;
    }
    player->state = PSTATE_USING_ITEM;
    player->state_timer = ITEM_USE_FRAMES;
}

void player_update(Player *player, const Screen *screen,
                   Projectile *projectiles, int *projectile_count, float dt) {
    if (player->attack_cooldown > 0) player->attack_cooldown--;
    if (player->invuln_timer > 0) player->invuln_timer--;

    if (player->state == PSTATE_KNOCKBACK) {
        player->knockback_timer--;
        float step = (float)KNOCKBACK_SPEED * dt;
        float nx = player->pos.x;
        float ny = player->pos.y;
        switch (player->knockback_dir) {
            case DIR_N: ny -= step; break;
            case DIR_S: ny += step; break;
            case DIR_W: nx -= step; break;
            case DIR_E: nx += step; break;
            default: break;
        }
        nx = Clamp(nx, 0, SCREEN_TILES_X * TILE_SIZE - TILE_SIZE);
        ny = Clamp(ny, PLAY_AREA_Y, PLAY_AREA_Y + SCREEN_TILES_Y * TILE_SIZE - TILE_SIZE);
        Rectangle test = { nx, ny, TILE_SIZE, TILE_SIZE };
        if (!screen_tile_blocked(screen, test)) {
            player->pos.x = nx;
            player->pos.y = ny;
        }
        if (player->knockback_timer <= 0) {
            player->state = PSTATE_IDLE;
        }
        anim_tick(&player->anim);
        return;
    }

    if (player->state == PSTATE_ATTACKING) {
        player->attack_timer--;
        if (player->attack_timer <= 0) {
            player->state = PSTATE_IDLE;
        }
        anim_tick(&player->anim);
        return;
    }

    if (player->state == PSTATE_USING_ITEM) {
        player->state_timer--;
        if (player->state_timer <= 0) {
            player->state = PSTATE_IDLE;
        }
        anim_tick(&player->anim);
        return;
    }

    if ((player->state == PSTATE_IDLE || player->state == PSTATE_MOVING) &&
        input_use_item() && player->attack_cooldown == 0) {
        try_use_item(player, projectiles, projectile_count);
        if (player->state == PSTATE_USING_ITEM) {
            player->attack_cooldown = ITEM_USE_FRAMES + SWORD_COOLDOWN_FRAMES;
            return;
        }
    }

    if ((player->state == PSTATE_IDLE || player->state == PSTATE_MOVING) &&
        input_attack() && player->attack_cooldown == 0) {
        player->state = PSTATE_ATTACKING;
        player->attack_timer = SWORD_ACTIVE_FRAMES;
        player->attack_cooldown = SWORD_ACTIVE_FRAMES + SWORD_COOLDOWN_FRAMES;
        anim_set(&player->anim, &player_attack_anims[player->facing]);
        return;
    }

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
    if (player->invuln_timer > 0 && (player->invuln_timer / 4) % 2 == 0) {
        return;
    }

    int frame = anim_frame_index(&player->anim);
    if (IsTextureValid(textures[TEX_PLAYER])) {
        Rectangle src = texture_frame_rect(4, frame);
        DrawTextureRec(textures[TEX_PLAYER], src, player->pos, WHITE);
    } else {
        DrawRectangle((int)player->pos.x, (int)player->pos.y, TILE_SIZE, TILE_SIZE, GREEN);
    }

    if (player->state == PSTATE_ATTACKING) {
        Rectangle sword = player_sword_hitbox(player);
        if (sword.width > 0) {
            DrawRectangle((int)sword.x, (int)sword.y,
                          (int)sword.width, (int)sword.height,
                          (Color){ 200, 200, 255, 160 });
        }
    }
}

Rectangle player_hitbox(const Player *player) {
    return (Rectangle){ player->pos.x, player->pos.y, TILE_SIZE, TILE_SIZE };
}

Rectangle player_sword_hitbox(const Player *player) {
    if (player->state != PSTATE_ATTACKING || player->attack_timer <= 0)
        return (Rectangle){ 0, 0, 0, 0 };
    float x = player->pos.x;
    float y = player->pos.y;
    switch (player->facing) {
        case DIR_N: y -= TILE_SIZE; break;
        case DIR_S: y += TILE_SIZE; break;
        case DIR_W: x -= TILE_SIZE; break;
        case DIR_E: x += TILE_SIZE; break;
        default: break;
    }
    return (Rectangle){ x, y, TILE_SIZE, TILE_SIZE };
}

void player_take_damage(Player *player, int damage, Direction kb_dir) {
    if (player->invuln_timer > 0) return;
    player->health -= damage;
    if (player->health < 0) player->health = 0;
    player->state = PSTATE_KNOCKBACK;
    player->knockback_timer = KNOCKBACK_FRAMES;
    player->knockback_dir = kb_dir;
    player->invuln_timer = INVULN_FRAMES;
}

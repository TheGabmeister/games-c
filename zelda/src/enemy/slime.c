#include "enemy.h"
#include "../game.h"
#include "../tilemap.h"
#include "../textures.h"
#include "../projectile.h"
#include "raymath.h"
#include <stdlib.h>

static Direction random_dir(void) {
    return (Direction)(rand() % DIR_COUNT);
}

static int random_range(int min, int max) {
    return min + rand() % (max - min + 1);
}

static void slime_update(Enemy *self, Vector2 player_pos, const Screen *screen,
                         Projectile *projectiles, int *projectile_count, float dt) {
    (void)projectiles; (void)projectile_count;
    (void)player_pos;
    const EnemyDef *def = &enemy_defs[self->type];

    switch (self->state) {
        case ESTATE_IDLE:
            self->state_timer--;
            if (self->state_timer <= 0) {
                self->facing = random_dir();
                self->velocity = (Vector2){ 0, 0 };
                switch (self->facing) {
                    case DIR_N: self->velocity.y = -1; break;
                    case DIR_S: self->velocity.y =  1; break;
                    case DIR_W: self->velocity.x = -1; break;
                    case DIR_E: self->velocity.x =  1; break;
                    default: break;
                }
                self->state = ESTATE_MOVING;
                self->state_timer = random_range(20, 40);
            }
            break;

        case ESTATE_MOVING: {
            float speed = def->speed;
            float nx = self->pos.x + self->velocity.x * speed * dt;
            float ny = self->pos.y + self->velocity.y * speed * dt;

            nx = Clamp(nx, 0, SCREEN_TILES_X * TILE_SIZE - TILE_SIZE);
            ny = Clamp(ny, PLAY_AREA_Y, PLAY_AREA_Y + SCREEN_TILES_Y * TILE_SIZE - TILE_SIZE);

            int w = def->hitbox_w;
            int h = def->hitbox_h;
            int ox = (TILE_SIZE - w) / 2;
            int oy = (TILE_SIZE - h) / 2;
            Rectangle test = { nx + ox, ny + oy, (float)w, (float)h };

            if (!screen_tile_blocked(screen, test)) {
                self->pos.x = nx;
                self->pos.y = ny;
            } else {
                self->state = ESTATE_IDLE;
                self->state_timer = random_range(30, 60);
                break;
            }

            self->state_timer--;
            if (self->state_timer <= 0) {
                self->state = ESTATE_IDLE;
                self->state_timer = random_range(30, 60);
            }
            break;
        }

        default:
            self->state = ESTATE_IDLE;
            self->state_timer = random_range(30, 60);
            break;
    }
}

static void slime_draw(const Enemy *self) {
    Color tint = WHITE;
    if (self->invuln_timer > 0 && (self->invuln_timer / 3) % 2 == 0)
        tint = (Color){ 255, 100, 100, 255 };

    int frame = (self->state == ESTATE_MOVING) ? 1 : 0;
    int row = (self->subtype == 1) ? 1 : 0;
    int frame_index = row * 4 + frame;

    if (IsTextureValid(textures[TEX_SLIME])) {
        Rectangle src = texture_frame_rect(4, frame_index);
        DrawTextureRec(textures[TEX_SLIME], src, self->pos, tint);
    } else {
        const EnemyDef *def = &enemy_defs[self->type];
        Color c = (self->subtype == 1) ? (Color){ 100, 220, 100, 255 }
                                       : (Color){ 50, 180, 50, 255 };
        if (self->invuln_timer > 0 && (self->invuln_timer / 3) % 2 == 0) c = WHITE;
        int w = def->hitbox_w;
        int h = def->hitbox_h;
        int ox = (TILE_SIZE - w) / 2;
        int oy = (TILE_SIZE - h) / 2;
        DrawRectangle((int)self->pos.x + ox, (int)self->pos.y + oy, w, h, c);
    }
}

static bool slime_on_hit(Enemy *self, Game *game, int damage) {
    if (self->subtype != 0 || game->player.inventory.sword_tier > 1) return false;
    Vector2 split_pos = self->pos;
    self->active = false;
    self->state = ESTATE_DEAD;
    for (int s = 0; s < 2 && game->enemy_count < MAX_ENEMIES_PER_SCREEN; s++) {
        Enemy *e = &game->enemies[game->enemy_count++];
        *e = (Enemy){0};
        e->type = ENEMY_SLIME;
        e->subtype = 1;
        e->pos.x = split_pos.x + (s == 0 ? -16.0f : 16.0f);
        e->pos.y = split_pos.y;
        e->pos.x = Clamp(e->pos.x, 0, SCREEN_TILES_X * TILE_SIZE - TILE_SIZE);
        e->facing = DIR_S;
        e->state = ESTATE_IDLE;
        e->state_timer = 20 + rand() % 40;
        e->health = 1;
        e->active = true;
    }
    (void)damage;
    return true;
}

EnemyDef slime_def(void) {
    return (EnemyDef){ "slime", 1, 1, 0, 96.0f, false, TILE_SIZE, TILE_SIZE, 0, slime_update, slime_draw, NULL, NULL, slime_on_hit };
}

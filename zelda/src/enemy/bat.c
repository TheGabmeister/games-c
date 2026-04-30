#include "enemy.h"
#include "../tilemap.h"
#include "../textures.h"
#include "../projectile.h"
#include "raymath.h"
#include <stdlib.h>
#include <math.h>

static int random_range(int min, int max) {
    return min + rand() % (max - min + 1);
}

static void pick_random_velocity(Enemy *self, float speed) {
    float angle = (float)(rand() % 360) * (3.14159f / 180.0f);
    self->velocity.x = cosf(angle) * speed;
    self->velocity.y = sinf(angle) * speed;
}

static void bat_update(Enemy *self, Vector2 player_pos, const Screen *screen,
                       Projectile *projectiles, int *projectile_count, float dt) {
    (void)projectiles; (void)projectile_count;
    (void)player_pos;
    (void)screen;
    const EnemyDef *def = &enemy_defs[self->type];

    switch (self->state) {
        case ESTATE_IDLE:
            self->state_timer--;
            if (self->state_timer <= 0) {
                self->state = ESTATE_MOVING;
                self->state_timer = random_range(120, 180);
                pick_random_velocity(self, def->speed);
                self->ai_timer = random_range(15, 25);
            }
            break;

        case ESTATE_MOVING: {
            self->ai_timer--;
            if (self->ai_timer <= 0) {
                pick_random_velocity(self, def->speed);
                self->ai_timer = random_range(15, 25);
            }

            float nx = self->pos.x + self->velocity.x * dt;
            float ny = self->pos.y + self->velocity.y * dt;

            float min_x = 0;
            float max_x = SCREEN_TILES_X * TILE_SIZE - TILE_SIZE;
            float min_y = PLAY_AREA_Y;
            float max_y = PLAY_AREA_Y + SCREEN_TILES_Y * TILE_SIZE - TILE_SIZE;

            if (nx < min_x || nx > max_x) {
                self->velocity.x = -self->velocity.x;
                nx = Clamp(nx, min_x, max_x);
            }
            if (ny < min_y || ny > max_y) {
                self->velocity.y = -self->velocity.y;
                ny = Clamp(ny, min_y, max_y);
            }
            self->pos.x = nx;
            self->pos.y = ny;

            self->state_timer--;
            if (self->state_timer <= 0) {
                self->state = ESTATE_CHARGING;
                self->state_timer = 30;
            }
            break;
        }

        case ESTATE_CHARGING: {
            float t = (float)self->state_timer / 30.0f;
            float nx = self->pos.x + self->velocity.x * t * dt;
            float ny = self->pos.y + self->velocity.y * t * dt;

            float min_x = 0;
            float max_x = SCREEN_TILES_X * TILE_SIZE - TILE_SIZE;
            float min_y = PLAY_AREA_Y;
            float max_y = PLAY_AREA_Y + SCREEN_TILES_Y * TILE_SIZE - TILE_SIZE;
            self->pos.x = Clamp(nx, min_x, max_x);
            self->pos.y = Clamp(ny, min_y, max_y);

            self->state_timer--;
            if (self->state_timer <= 0) {
                self->state = ESTATE_IDLE;
                self->state_timer = random_range(60, 120);
                self->velocity = (Vector2){ 0, 0 };
            }
            break;
        }

        default:
            self->state = ESTATE_IDLE;
            self->state_timer = random_range(60, 120);
            break;
    }
}

static void bat_draw(const Enemy *self) {
    Color tint = WHITE;
    if (self->invuln_timer > 0 && (self->invuln_timer / 3) % 2 == 0)
        tint = (Color){ 255, 100, 100, 255 };

    int frame;
    if (self->state == ESTATE_IDLE) {
        frame = 0;
    } else if (self->state == ESTATE_MOVING) {
        frame = 1 + (self->ai_timer % 3);
    } else {
        frame = 5;
    }

    if (IsTextureValid(textures[TEX_BAT])) {
        Rectangle src = texture_frame_rect(4, frame);
        DrawTextureRec(textures[TEX_BAT], src, self->pos, tint);
    } else {
        Color c = (Color){ 160, 50, 200, 255 };
        if (self->invuln_timer > 0 && (self->invuln_timer / 3) % 2 == 0) c = WHITE;
        DrawRectangle((int)self->pos.x, (int)self->pos.y, TILE_SIZE, TILE_SIZE, c);
    }
}

EnemyDef bat_def(void) {
    return (EnemyDef){ "bat", 1, 1, 0, 160.0f, true, TILE_SIZE, TILE_SIZE, 0, bat_update, bat_draw, NULL, NULL, NULL };
}

#include "enemy.h"
#include "../tilemap.h"
#include "../textures.h"
#include "../projectile.h"
#include "raymath.h"
#include <stdlib.h>
#include <math.h>

#define SNAKE_CHARGE_SPEED 352.0f

static Direction random_dir(void) {
    return (Direction)(rand() % DIR_COUNT);
}

static int random_range(int min, int max) {
    return min + rand() % (max - min + 1);
}

static void set_velocity_from_dir(Enemy *self, float speed) {
    self->velocity = (Vector2){ 0, 0 };
    switch (self->facing) {
        case DIR_N: self->velocity.y = -speed; break;
        case DIR_S: self->velocity.y =  speed; break;
        case DIR_W: self->velocity.x = -speed; break;
        case DIR_E: self->velocity.x =  speed; break;
        default: break;
    }
}

void snake_update(Enemy *self, Vector2 player_pos, const Screen *screen,
                  Projectile *projectiles, int *projectile_count, float dt) {
    (void)projectiles; (void)projectile_count;
    const EnemyDef *def = &enemy_defs[self->type];

    switch (self->state) {
        case ESTATE_IDLE: {
            float speed = def->speed;
            float nx = self->pos.x + self->velocity.x * dt;
            float ny = self->pos.y + self->velocity.y * dt;

            nx = Clamp(nx, 0, SCREEN_TILES_X * TILE_SIZE - TILE_SIZE);
            ny = Clamp(ny, PLAY_AREA_Y, PLAY_AREA_Y + SCREEN_TILES_Y * TILE_SIZE - TILE_SIZE);

            Rectangle test = { nx, ny, TILE_SIZE, TILE_SIZE };
            if (!screen_tile_blocked(screen, test)) {
                self->pos.x = nx;
                self->pos.y = ny;
            } else {
                self->facing = random_dir();
                set_velocity_from_dir(self, speed);
                self->state_timer = random_range(60, 120);
            }

            self->state_timer--;
            if (self->state_timer <= 0) {
                self->facing = random_dir();
                set_velocity_from_dir(self, speed);
                self->state_timer = random_range(60, 120);
            }

            float dx = player_pos.x - self->pos.x;
            float dy = player_pos.y - self->pos.y;
            float half = TILE_SIZE / 2.0f;

            if (fabsf(dy) < half && fabsf(dx) > half) {
                self->facing = (dx > 0) ? DIR_E : DIR_W;
                set_velocity_from_dir(self, SNAKE_CHARGE_SPEED);
                self->state = ESTATE_CHARGING;
            } else if (fabsf(dx) < half && fabsf(dy) > half) {
                self->facing = (dy > 0) ? DIR_S : DIR_N;
                set_velocity_from_dir(self, SNAKE_CHARGE_SPEED);
                self->state = ESTATE_CHARGING;
            }
            break;
        }

        case ESTATE_CHARGING: {
            float nx = self->pos.x + self->velocity.x * dt;
            float ny = self->pos.y + self->velocity.y * dt;

            nx = Clamp(nx, 0, SCREEN_TILES_X * TILE_SIZE - TILE_SIZE);
            ny = Clamp(ny, PLAY_AREA_Y, PLAY_AREA_Y + SCREEN_TILES_Y * TILE_SIZE - TILE_SIZE);

            Rectangle test = { nx, ny, TILE_SIZE, TILE_SIZE };
            if (!screen_tile_blocked(screen, test)) {
                self->pos.x = nx;
                self->pos.y = ny;
            } else {
                self->state = ESTATE_IDLE;
                self->facing = random_dir();
                set_velocity_from_dir(self, def->speed);
                self->state_timer = random_range(60, 120);
            }

            bool at_edge = (self->pos.x <= 0 ||
                            self->pos.x >= SCREEN_TILES_X * TILE_SIZE - TILE_SIZE ||
                            self->pos.y <= PLAY_AREA_Y ||
                            self->pos.y >= PLAY_AREA_Y + SCREEN_TILES_Y * TILE_SIZE - TILE_SIZE);
            if (at_edge) {
                self->state = ESTATE_IDLE;
                self->facing = random_dir();
                set_velocity_from_dir(self, def->speed);
                self->state_timer = random_range(60, 120);
            }
            break;
        }

        default:
            self->state = ESTATE_IDLE;
            self->facing = random_dir();
            set_velocity_from_dir(self, def->speed);
            self->state_timer = random_range(60, 120);
            break;
    }
}

void snake_draw(const Enemy *self) {
    Color tint = WHITE;
    if (self->invuln_timer > 0 && (self->invuln_timer / 3) % 2 == 0)
        tint = (Color){ 255, 100, 100, 255 };

    int dir_index;
    switch (self->facing) {
        case DIR_S: dir_index = 0; break;
        case DIR_N: dir_index = 1; break;
        case DIR_E: dir_index = 2; break;
        case DIR_W: dir_index = 3; break;
        default:    dir_index = 0; break;
    }
    int row = (self->state == ESTATE_CHARGING) ? 1 : 0;
    int frame = row * 4 + dir_index;

    if (IsTextureValid(textures[TEX_SNAKE])) {
        Rectangle src = texture_frame_rect(4, frame);
        DrawTextureRec(textures[TEX_SNAKE], src, self->pos, tint);
    } else {
        Color c = (self->state == ESTATE_CHARGING)
            ? (Color){ 255, 80, 80, 255 }
            : (Color){ 200, 60, 60, 255 };
        if (self->invuln_timer > 0 && (self->invuln_timer / 3) % 2 == 0) c = WHITE;
        DrawRectangle((int)self->pos.x, (int)self->pos.y, TILE_SIZE, TILE_SIZE, c);
    }
}

#include "enemy.h"
#include "../tilemap.h"
#include "../textures.h"
#include "../projectile.h"
#include "raymath.h"
#include <stdlib.h>
#include <math.h>

static Direction random_dir(void) {
    return (Direction)(rand() % DIR_COUNT);
}

static void face_toward_player(Enemy *self, Vector2 player_pos) {
    float dx = player_pos.x - self->pos.x;
    float dy = player_pos.y - self->pos.y;
    if (fabsf(dx) > fabsf(dy))
        self->facing = (dx > 0) ? DIR_E : DIR_W;
    else
        self->facing = (dy > 0) ? DIR_S : DIR_N;
}

static void spear_thrower_update(Enemy *self, Vector2 player_pos, const Screen *screen,
                                 Projectile *projectiles, int *projectile_count, float dt) {
    const EnemyDef *def = &enemy_defs[self->type];

    switch (self->state) {
        case ESTATE_IDLE:
            self->state_timer--;
            if (self->state_timer <= 0) {
                if (rand() % 2 == 0) {
                    face_toward_player(self, player_pos);
                    self->state = ESTATE_CHARGING;
                    self->state_timer = 15;
                } else {
                    self->facing = random_dir();
                    self->state = ESTATE_MOVING;
                    self->state_timer = 30 + rand() % 60;
                }
            }
            break;

        case ESTATE_MOVING: {
            float step = def->speed * dt;
            float nx = self->pos.x;
            float ny = self->pos.y;
            switch (self->facing) {
                case DIR_N: ny -= step; break;
                case DIR_S: ny += step; break;
                case DIR_W: nx -= step; break;
                case DIR_E: nx += step; break;
                default: break;
            }
            nx = Clamp(nx, 0, SCREEN_TILES_X * TILE_SIZE - TILE_SIZE);
            ny = Clamp(ny, PLAY_AREA_Y, PLAY_AREA_Y + SCREEN_TILES_Y * TILE_SIZE - TILE_SIZE);
            Rectangle test = { nx, ny, (float)TILE_SIZE, (float)TILE_SIZE };
            if (!screen_tile_blocked(screen, test)) {
                self->pos.x = nx;
                self->pos.y = ny;
            } else {
                self->facing = random_dir();
            }
            self->state_timer--;
            if (self->state_timer <= 0) {
                self->state = ESTATE_IDLE;
                self->state_timer = 30 + rand() % 60;
            }
            break;
        }

        case ESTATE_CHARGING:
            self->state_timer--;
            if (self->state_timer <= 0) {
                projectile_spawn(projectiles, projectile_count,
                                 PROJ_SPEAR, OWNER_ENEMY, self->pos, self->facing);
                self->state = ESTATE_IDLE;
                self->state_timer = 40 + rand() % 40;
            }
            break;

        default:
            self->state = ESTATE_IDLE;
            self->state_timer = 30 + rand() % 60;
            break;
    }
}

static void spear_thrower_draw(const Enemy *self) {
    Color tint = (Color){ 180, 140, 60, 255 };
    if (self->invuln_timer > 0 && (self->invuln_timer / 3) % 2 == 0)
        tint = (Color){ 255, 100, 100, 255 };

    if (self->state == ESTATE_CHARGING) {
        tint.r = (unsigned char)(tint.r + 40 > 255 ? 255 : tint.r + 40);
    }

    DrawRectangle((int)self->pos.x + 12, (int)self->pos.y + 4, 40, 56, tint);
    DrawRectangle((int)self->pos.x + 20, (int)self->pos.y + 16, 10, 10, DARKBROWN);
    DrawRectangle((int)self->pos.x + 34, (int)self->pos.y + 16, 10, 10, DARKBROWN);
}

EnemyDef spear_thrower_def(void) {
    return (EnemyDef){ "spear_thrower", 2, 1, 0, 80.0f, false, TILE_SIZE, TILE_SIZE, 0, spear_thrower_update, spear_thrower_draw, NULL, NULL, NULL };
}

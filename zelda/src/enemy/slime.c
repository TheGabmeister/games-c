#include "enemy.h"
#include "../tilemap.h"
#include "../textures.h"
#include <stdlib.h>

static Direction random_dir(void) {
    return (Direction)(rand() % DIR_COUNT);
}

static int random_range(int min, int max) {
    return min + rand() % (max - min + 1);
}

void slime_update(Enemy *self, Vector2 player_pos, const Screen *screen, float dt) {
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

void slime_draw(const Enemy *self) {
    Color tint = WHITE;
    if (self->invuln_timer > 0 && (self->invuln_timer / 3) % 2 == 0)
        tint = (Color){ 255, 100, 100, 255 };

    int frame = (self->state == ESTATE_MOVING) ? 1 : 0;
    int row = (self->subtype == 1) ? 1 : 0;
    int frame_index = row * 4 + frame;

    if (IsTextureValid(textures[TEX_ENEMIES])) {
        Rectangle src = texture_frame_rect(4, frame_index);
        DrawTextureRec(textures[TEX_ENEMIES], src, self->pos, tint);
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

#include "enemy.h"
#include "../game.h"
#include "../sounds.h"
#include "../tilemap.h"
#include "../textures.h"
#include "../projectile.h"
#include "../game_config.h"
#include "raymath.h"
#include <math.h>
#include <stdlib.h>

#define DRAGON_SPEED       64.0f
#define DRAGON_WINDUP_TIME 30
#define DRAGON_FIRE_TIME   15
#define DRAGON_COOLDOWN_MIN 60
#define DRAGON_COOLDOWN_MAX 90

// Dragon reuses EnemyState values: subtype encodes the dragon-specific sub-state
// ai_timer > 0: patrol; ai_timer == 0 and state_timer > 0: windup/fire/cooldown
// subtype: 0=patrol, 1=windup, 2=fire, 3=cooldown
#define DSUB_PATROL   0
#define DSUB_WINDUP   1
#define DSUB_FIRE     2
#define DSUB_COOLDOWN 3

void dragon_update(Enemy *self, Vector2 player_pos, const Screen *screen,
                   Projectile *projectiles, int *projectile_count, float dt) {
    (void)screen;

    switch (self->subtype) {
        case DSUB_PATROL: {
            float nx = self->pos.x + self->velocity.x * dt;
            float min_x = (float)TILE_SIZE;
            float max_x = (float)((SCREEN_TILES_X - 3) * TILE_SIZE);
            if (nx <= min_x || nx >= max_x) {
                self->velocity.x = -self->velocity.x;
                nx = Clamp(nx, min_x, max_x);
            }
            self->pos.x = nx;
            self->facing = (self->velocity.x > 0) ? DIR_E : DIR_W;

            self->ai_timer--;
            if (self->ai_timer <= 0) {
                self->subtype = DSUB_WINDUP;
                self->state_timer = DRAGON_WINDUP_TIME;
                self->velocity.x = 0;
            }
            break;
        }

        case DSUB_WINDUP:
            self->state_timer--;
            if (self->state_timer <= 0) {
                self->subtype = DSUB_FIRE;
                self->state_timer = DRAGON_FIRE_TIME;

                float cx = self->pos.x + TILE_SIZE;
                float cy = self->pos.y + TILE_SIZE;
                float dx = player_pos.x + TILE_SIZE / 2.0f - cx;
                float dy = player_pos.y + TILE_SIZE / 2.0f - cy;
                Direction beam_dir;
                if (fabsf(dx) > fabsf(dy))
                    beam_dir = (dx > 0) ? DIR_E : DIR_W;
                else
                    beam_dir = (dy > 0) ? DIR_S : DIR_N;

                self->facing = beam_dir;
                Vector2 beam_pos = self->pos;
                switch (beam_dir) {
                    case DIR_N: beam_pos.x += TILE_SIZE / 2; beam_pos.y -= TILE_SIZE; break;
                    case DIR_S: beam_pos.x += TILE_SIZE / 2; beam_pos.y += TILE_SIZE * 2; break;
                    case DIR_E: beam_pos.x += TILE_SIZE * 2; beam_pos.y += TILE_SIZE / 2; break;
                    case DIR_W: beam_pos.x -= TILE_SIZE;     beam_pos.y += TILE_SIZE / 2; break;
                    default: break;
                }

                projectile_spawn(projectiles, projectile_count,
                                 PROJ_DRAGON_BEAM, OWNER_ENEMY, beam_pos, beam_dir);
            }
            break;

        case DSUB_FIRE:
            self->state_timer--;
            if (self->state_timer <= 0) {
                self->subtype = DSUB_COOLDOWN;
                self->state_timer = DRAGON_COOLDOWN_MIN + rand() % (DRAGON_COOLDOWN_MAX - DRAGON_COOLDOWN_MIN);
            }
            break;

        case DSUB_COOLDOWN:
            self->state_timer--;
            if (self->state_timer <= 0) {
                self->subtype = DSUB_PATROL;
                self->velocity.x = (rand() % 2 == 0) ? DRAGON_SPEED : -DRAGON_SPEED;
                self->ai_timer = 90 + rand() % 60;
            }
            break;
    }
}

void dragon_draw(const Enemy *self) {
    int size = TILE_SIZE * 2;
    bool flash = (self->invuln_timer > 0 && (self->invuln_timer / 3) % 2 == 0);
    bool windup_flash = (self->subtype == DSUB_WINDUP && (self->state_timer / 4) % 2 == 0);

    Color body_color = { 40, 160, 40, 255 };
    Color head_color = { 60, 200, 60, 255 };

    if (flash) {
        body_color = WHITE;
        head_color = WHITE;
    } else if (windup_flash) {
        body_color = (Color){ 200, 60, 60, 255 };
        head_color = (Color){ 240, 80, 80, 255 };
    }

    DrawRectangle((int)self->pos.x, (int)self->pos.y, size, size, body_color);

    int hx = (int)self->pos.x + TILE_SIZE / 2;
    int hy = (int)self->pos.y;
    switch (self->facing) {
        case DIR_N: hx = (int)self->pos.x + TILE_SIZE / 2; hy = (int)self->pos.y - 16; break;
        case DIR_S: hx = (int)self->pos.x + TILE_SIZE / 2; hy = (int)self->pos.y + size; break;
        case DIR_E: hx = (int)self->pos.x + size;          hy = (int)self->pos.y + TILE_SIZE / 2; break;
        case DIR_W: hx = (int)self->pos.x - TILE_SIZE / 2; hy = (int)self->pos.y + TILE_SIZE / 2; break;
        default: break;
    }
    DrawRectangle(hx, hy, TILE_SIZE, TILE_SIZE / 2, head_color);

    if (self->subtype == DSUB_FIRE) {
        DrawCircle((int)self->pos.x + TILE_SIZE, (int)self->pos.y + TILE_SIZE, 8.0f, ORANGE);
    }
}

void dragon_on_spawn(Enemy *self) {
    self->velocity.x = (rand() % 2 == 0) ? DRAGON_SPEED : -DRAGON_SPEED;
    self->ai_timer = 90 + rand() % 60;
}

void dragon_on_death(Enemy *self, Game *game) {
    (void)self;
    if (!game->in_dungeon || !game->current_screen.is_boss_room) return;
    if (game->dungeon.boss_defeated) return;
    game->dungeon.boss_defeated = true;
    sound_play(SOUND_BOSS_DEFEAT);
}

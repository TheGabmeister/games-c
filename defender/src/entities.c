#include "game.h"
#include "sounds.h"

#include <math.h>

static void fire_laser(Game *game) {
    if (game->player.fire_timer > 0.0f) return;

    for (int i = 0; i < MAX_LASERS; i++) {
        Laser *laser = &game->lasers[i];
        if (!laser->active) {
            laser->x = wrap_x(game->player.x + (float)game->player.facing * 24.0f);
            laser->y = game->player.y;
            laser->vx = (float)game->player.facing * LASER_SPEED;
            laser->life = LASER_LIFETIME;
            laser->facing = game->player.facing;
            laser->active = true;
            game->player.fire_timer = PLAYER_FIRE_COOLDOWN;
            sound_play(game, SND_SHOOT);
            return;
        }
    }
}

static void enemy_fire(Game *game, const Enemy *enemy) {
    for (int i = 0; i < MAX_BULLETS; i++) {
        EnemyBullet *b = &game->bullets[i];
        if (!b->active) {
            float dx = wrapped_delta(enemy->x, game->player.x);
            float dy = game->player.y - enemy->y;
            float len = sqrtf(dx * dx + dy * dy);
            if (len < 1.0f) len = 1.0f;

            b->x = enemy->x;
            b->y = enemy->y;
            b->vx = dx / len * BULLET_SPEED;
            b->vy = dy / len * BULLET_SPEED;
            b->life = BULLET_LIFETIME;
            b->active = true;
            return;
        }
    }
}

static int nearest_safe_humanoid(Game *game, float x) {
    int best = -1;
    float best_dist = WORLD_WIDTH;
    for (int i = 0; i < MAX_HUMANOIDS; i++) {
        Humanoid *h = &game->humanoids[i];
        if (h->state != HUMANOID_SAFE) continue;

        float dist = fabsf(wrapped_delta(x, h->x));
        if (dist < best_dist) {
            best = i;
            best_dist = dist;
        }
    }
    return best;
}

static void spawn_pressure_enemy(Game *game, EnemyKind kind, float x, float y) {
    for (int i = 0; i < MAX_ENEMIES; i++) {
        Enemy *e = &game->enemies[i];
        if (!e->active) {
            e->x = wrap_x(x);
            e->y = y;
            e->vx = randf(-60.0f, 60.0f);
            e->vy = randf(-20.0f, 20.0f);
            e->phase = randf(0.0f, PI_F * 2.0f);
            e->fire_timer = randf(0.4f, ENEMY_FIRE_COOLDOWN);
            e->target_humanoid = -1;
            e->carrying = false;
            e->kind = kind;
            e->active = true;
            game->enemy_count++;
            return;
        }
    }
}

static void kill_enemy(Game *game, int index, bool by_smart_bomb) {
    Enemy *enemy = &game->enemies[index];
    if (!enemy->active) return;

    if (enemy->carrying && enemy->target_humanoid >= 0) {
        Humanoid *h = &game->humanoids[enemy->target_humanoid];
        if (h->state == HUMANOID_ABDUCTED) {
            h->state = HUMANOID_FALLING;
            h->carrier = -1;
            h->vy = 0.0f;
        }
    }

    int points = enemy->kind == ENEMY_MUTANT ? SCORE_MUTANT : SCORE_LANDER;
    if (enemy->kind == ENEMY_BAITER) points = SCORE_MUTANT + 100;
    add_score(game, points);
    burst(game, enemy->x, enemy->y, enemy->kind == ENEMY_MUTANT ? COLOR_MUTANT : COLOR_LANDER, by_smart_bomb ? 28 : 18, by_smart_bomb ? 360.0f : 260.0f);
    enemy->active = false;
    game->screen_shake = fmaxf(game->screen_shake, by_smart_bomb ? 0.25f : 0.12f);
    sound_play(game, SND_EXPLOSION);
}

static void player_die(Game *game) {
    if (game->state == STATE_DYING || game->player.invuln_timer > 0.0f) return;

    game->lives--;
    game->player.alive = false;
    game->state = STATE_DYING;
    game->state_timer = PLAYER_RESPAWN_TIME;
    game->screen_shake = 0.55f;
    burst(game, game->player.x, game->player.y, COLOR_PLAYER, 48, 420.0f);
    sound_play(game, SND_PLAYER_DEATH);
}

static void use_smart_bomb(Game *game) {
    if (game->smart_bombs <= 0) return;
    game->smart_bombs--;
    game->screen_shake = 0.5f;
    sound_play(game, SND_SMART_BOMB);

    for (int i = 0; i < MAX_ENEMIES; i++) {
        Enemy *enemy = &game->enemies[i];
        if (!enemy->active) continue;
        if (fabsf(wrapped_delta(game->player.x, enemy->x)) < SMART_BOMB_RADIUS) {
            kill_enemy(game, i, true);
        }
    }

    for (int i = 0; i < 90; i++) {
        float x = wrap_x(game->player.x + randf(-WINDOW_WIDTH * 0.5f, WINDOW_WIDTH * 0.5f));
        float y = randf(100.0f, 590.0f);
        spawn_particle(game, x, y, randf(-250.0f, 250.0f), randf(-180.0f, 180.0f), randf(2.0f, 7.0f), randf(0.3f, 0.85f), COLOR_LASER);
    }
}

static void use_hyperspace(Game *game) {
    sound_play(game, SND_HYPERSPACE);
    burst(game, game->player.x, game->player.y, COLOR_PLAYER, 20, 240.0f);

    if (GetRandomValue(1, 100) <= HYPERSPACE_DEATH_CHANCE) {
        player_die(game);
        return;
    }

    game->player.x = randf(0.0f, WORLD_WIDTH);
    game->player.y = randf(SKY_TOP + 80.0f, TERRAIN_BASE_Y - 120.0f);
    game->player.vx *= 0.35f;
    game->player.vy *= 0.35f;
    game->camera_x = game->player.x;
    game->screen_shake = 0.18f;
    burst(game, game->player.x, game->player.y, COLOR_PLAYER, 24, 280.0f);
}

void update_player(Game *game, float dt) {
    PlayerShip *p = &game->player;
    if (!p->alive) return;

    float ax = 0.0f;
    float ay = 0.0f;
    if (IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A)) ax -= PLAYER_ACCEL;
    if (IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D)) ax += PLAYER_ACCEL;
    if (IsKeyDown(KEY_UP) || IsKeyDown(KEY_W)) ay -= PLAYER_ACCEL;
    if (IsKeyDown(KEY_DOWN) || IsKeyDown(KEY_S)) ay += PLAYER_ACCEL;

    if (ax < 0.0f) p->facing = FACE_LEFT;
    if (ax > 0.0f) p->facing = FACE_RIGHT;
    if (IsKeyPressed(KEY_LEFT_SHIFT) || IsKeyPressed(KEY_RIGHT_SHIFT)) p->facing = (Facing)(-(int)p->facing);
    if (IsKeyPressed(KEY_SPACE)) fire_laser(game);
    if (IsKeyPressed(KEY_B)) use_smart_bomb(game);
    if (IsKeyPressed(KEY_H)) use_hyperspace(game);

    p->vx += ax * dt;
    p->vy += ay * dt;
    p->vx *= powf(PLAYER_DRAG, dt * 60.0f);
    p->vy *= powf(PLAYER_DRAG, dt * 60.0f);
    p->vx = clampf(p->vx, -PLAYER_MAX_SPEED_X, PLAYER_MAX_SPEED_X);
    p->vy = clampf(p->vy, -PLAYER_MAX_SPEED_Y, PLAYER_MAX_SPEED_Y);

    p->x = wrap_x(p->x + p->vx * dt);
    p->y += p->vy * dt;

    float ground = terrain_height_at(&game->terrain, p->x) - 42.0f;
    p->y = clampf(p->y, SKY_TOP + 30.0f, ground);
    if (p->fire_timer > 0.0f) p->fire_timer -= dt;
    if (p->invuln_timer > 0.0f) p->invuln_timer -= dt;

    game->camera_x = wrap_x(game->camera_x + wrapped_delta(game->camera_x, p->x) * fminf(dt * 7.5f, 1.0f));
    game->star_scroll += p->vx * dt * 0.02f;
}

void update_lasers(Game *game, float dt) {
    for (int i = 0; i < MAX_LASERS; i++) {
        Laser *laser = &game->lasers[i];
        if (!laser->active) continue;

        laser->x = wrap_x(laser->x + laser->vx * dt);
        laser->life -= dt;
        if (laser->life <= 0.0f) {
            laser->active = false;
            continue;
        }

        for (int e = 0; e < MAX_ENEMIES; e++) {
            Enemy *enemy = &game->enemies[e];
            if (!enemy->active) continue;

            float dx = fabsf(wrapped_delta(laser->x, enemy->x));
            float dy = fabsf(laser->y - enemy->y);
            if (dx < LASER_LENGTH * 0.55f && dy < LASER_HIT_RADIUS) {
                laser->active = false;
                kill_enemy(game, e, false);
                break;
            }
        }
    }
}

void update_bullets(Game *game, float dt) {
    for (int i = 0; i < MAX_BULLETS; i++) {
        EnemyBullet *b = &game->bullets[i];
        if (!b->active) continue;

        b->x = wrap_x(b->x + b->vx * dt);
        b->y += b->vy * dt;
        b->life -= dt;
        if (b->life <= 0.0f || b->y < SKY_TOP || b->y > terrain_height_at(&game->terrain, b->x)) {
            b->active = false;
            continue;
        }

        float dx = wrapped_delta(b->x, game->player.x);
        float dy = b->y - game->player.y;
        if (game->player.alive && dx * dx + dy * dy < PLAYER_RADIUS * PLAYER_RADIUS) {
            b->active = false;
            player_die(game);
        }
    }
}

void update_humanoids(Game *game, float dt) {
    for (int i = 0; i < MAX_HUMANOIDS; i++) {
        Humanoid *h = &game->humanoids[i];
        if (h->state == HUMANOID_LOST) continue;

        h->ground_y = terrain_height_at(&game->terrain, h->x) - HUMANOID_RADIUS;

        if (h->state == HUMANOID_ABDUCTED) {
            if (h->carrier >= 0 && game->enemies[h->carrier].active) {
                h->x = game->enemies[h->carrier].x;
                h->y = game->enemies[h->carrier].y + 24.0f;
            } else {
                h->state = HUMANOID_FALLING;
                h->carrier = -1;
                h->vy = 0.0f;
            }
        } else if (h->state == HUMANOID_FALLING) {
            h->vy += HUMANOID_FALL_SPEED * dt;
            h->y += h->vy * dt;

            float dx = wrapped_delta(h->x, game->player.x);
            float dy = h->y - game->player.y;
            if (game->player.alive && dx * dx + dy * dy < HUMANOID_RESCUE_RADIUS * HUMANOID_RESCUE_RADIUS) {
                h->state = HUMANOID_RESCUED;
                h->vy = 0.0f;
                add_score(game, SCORE_RESCUE);
                sound_play(game, SND_RESCUE);
                burst(game, h->x, h->y, COLOR_HUMANOID, 14, 150.0f);
            } else if (h->y >= h->ground_y) {
                h->state = HUMANOID_LOST;
                h->y = h->ground_y;
                game->humanoid_count--;
                sound_play(game, SND_HUMANOID_LOST);
                burst(game, h->x, h->y, COLOR_LASER, 14, 170.0f);
            }
        } else if (h->state == HUMANOID_RESCUED) {
            h->x = game->player.x;
            h->ground_y = terrain_height_at(&game->terrain, h->x) - HUMANOID_RADIUS;
            h->y = game->player.y + 26.0f;
            if (h->y >= h->ground_y - HUMANOID_PICKUP_Y) {
                h->state = HUMANOID_SAFE;
                h->x = game->player.x;
                h->ground_y = terrain_height_at(&game->terrain, h->x) - HUMANOID_RADIUS;
                h->y = h->ground_y;
                burst(game, h->x, h->y, COLOR_HUMANOID, 12, 110.0f);
            }
        }
    }
}

static void mutate_all_landers(Game *game) {
    for (int i = 0; i < MAX_ENEMIES; i++) {
        Enemy *e = &game->enemies[i];
        if (e->active && e->kind == ENEMY_LANDER) {
            e->kind = ENEMY_MUTANT;
            e->target_humanoid = -1;
            e->carrying = false;
        }
    }
}

void update_enemy_ai(Game *game, float dt) {
    if (living_humanoids(game) == 0) {
        mutate_all_landers(game);
    }

    game->baiter_timer -= dt;
    if (game->baiter_timer <= 0.0f && living_enemies(game) > 0) {
        spawn_pressure_enemy(game, ENEMY_BAITER, wrap_x(game->player.x + (float)GetRandomValue(-1, 1) * 700.0f), randf(120.0f, 280.0f));
        game->baiter_timer = 26.0f;
    }

    for (int i = 0; i < MAX_ENEMIES; i++) {
        Enemy *e = &game->enemies[i];
        if (!e->active) continue;

        e->phase += dt * 3.0f;
        float target_x = game->player.x;
        float target_y = game->player.y;
        float speed = LANDER_BASE_SPEED + game->wave * 7.0f;

        if (e->kind == ENEMY_LANDER) {
            if (!e->carrying) {
                if (e->target_humanoid < 0 || game->humanoids[e->target_humanoid].state != HUMANOID_SAFE) {
                    e->target_humanoid = nearest_safe_humanoid(game, e->x);
                }

                if (e->target_humanoid >= 0) {
                    Humanoid *h = &game->humanoids[e->target_humanoid];
                    target_x = h->x;
                    target_y = h->y - 20.0f;

                    if (fabsf(wrapped_delta(e->x, h->x)) < 12.0f && fabsf(e->y - target_y) < 16.0f) {
                        e->carrying = true;
                        h->state = HUMANOID_ABDUCTED;
                        h->carrier = i;
                    }
                } else {
                    target_x = wrap_x(e->x + sinf(e->phase) * 180.0f);
                    target_y = 230.0f + sinf(e->phase * 0.6f) * 90.0f;
                }
            } else {
                target_y = SKY_TOP + 12.0f;
                target_x = e->x + sinf(e->phase) * 24.0f;
                speed *= 0.88f;

                if (e->y <= SKY_TOP + 18.0f) {
                    if (e->target_humanoid >= 0) {
                        Humanoid *h = &game->humanoids[e->target_humanoid];
                        h->state = HUMANOID_LOST;
                        h->carrier = -1;
                        game->humanoid_count--;
                        sound_play(game, SND_HUMANOID_LOST);
                    }
                    e->kind = ENEMY_MUTANT;
                    e->carrying = false;
                    e->target_humanoid = -1;
                    burst(game, e->x, e->y, COLOR_MUTANT, 22, 230.0f);
                }
            }
        } else {
            speed = (e->kind == ENEMY_BAITER ? MUTANT_BASE_SPEED + 85.0f : MUTANT_BASE_SPEED) + game->wave * 10.0f;
            target_x = game->player.x;
            target_y = game->player.y + sinf(e->phase * 2.2f) * 24.0f;
        }

        float dx = wrapped_delta(e->x, target_x);
        float dy = target_y - e->y;
        float len = sqrtf(dx * dx + dy * dy);
        if (len < 1.0f) len = 1.0f;

        e->vx = approach(e->vx, dx / len * speed, dt * speed * 2.5f);
        e->vy = approach(e->vy, dy / len * speed, dt * speed * 2.5f);
        e->x = wrap_x(e->x + e->vx * dt);
        e->y += e->vy * dt + sinf(e->phase * 3.0f) * 8.0f * dt;
        e->y = clampf(e->y, SKY_TOP + 10.0f, terrain_height_at(&game->terrain, e->x) - 42.0f);

        e->fire_timer -= dt;
        if (e->fire_timer <= 0.0f && near_camera(game, e->x, 180.0f) && e->kind != ENEMY_LANDER) {
            enemy_fire(game, e);
            e->fire_timer = ENEMY_FIRE_COOLDOWN + randf(0.0f, 1.1f);
        }

        float px = wrapped_delta(e->x, game->player.x);
        float py = e->y - game->player.y;
        float hit_radius = ENEMY_RADIUS + PLAYER_RADIUS;
        if (game->player.alive && px * px + py * py < hit_radius * hit_radius) {
            kill_enemy(game, i, false);
            player_die(game);
        }
    }
}

void update_particles(Game *game, float dt) {
    for (int i = 0; i < MAX_PARTICLES; i++) {
        Particle *p = &game->particles[i];
        if (!p->active) continue;

        p->x = wrap_x(p->x + p->vx * dt);
        p->y += p->vy * dt;
        p->vy += 45.0f * dt;
        p->life -= dt;
        if (p->life <= 0.0f) p->active = false;
    }
}

void check_wave_complete(Game *game) {
    if (living_enemies(game) > 0) return;

    int bonus = SCORE_WAVE_CLEAR;
    for (int i = 0; i < MAX_HUMANOIDS; i++) {
        if (game->humanoids[i].state != HUMANOID_LOST) bonus += SCORE_HUMANOID_SAFE;
    }
    add_score(game, bonus);
    game->wave++;
    game->state = STATE_WAVE_COMPLETE;
    game->state_timer = 2.0f;
    game->screen_shake = 0.18f;
    sound_play(game, SND_WAVE_CLEAR);
}

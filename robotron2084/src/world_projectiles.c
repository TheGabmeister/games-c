#include "world_internal.h"
#include "sounds.h"
#include "raymath.h"
#include <math.h>

static Vector2 get_move_input(void) {
    Vector2 move = { 0.0f, 0.0f };

    if (IsKeyDown(KEY_A)) move.x -= 1.0f;
    if (IsKeyDown(KEY_D)) move.x += 1.0f;
    if (IsKeyDown(KEY_W)) move.y -= 1.0f;
    if (IsKeyDown(KEY_S)) move.y += 1.0f;

    if (move.x != 0.0f || move.y != 0.0f) {
        move = Vector2Normalize(move);
    }

    return move;
}

static Vector2 get_fire_input(void) {
    Vector2 aim = { 0.0f, 0.0f };

    if (IsKeyDown(KEY_LEFT)) aim.x -= 1.0f;
    if (IsKeyDown(KEY_RIGHT)) aim.x += 1.0f;
    if (IsKeyDown(KEY_UP)) aim.y -= 1.0f;
    if (IsKeyDown(KEY_DOWN)) aim.y += 1.0f;

    if (aim.x != 0.0f || aim.y != 0.0f) {
        aim = Vector2Normalize(aim);
    }

    return aim;
}

static void spawn_bullet(Game *game, Vector2 direction) {
    for (int i = 0; i < MAX_BULLETS; i++) {
        Bullet *bullet = &game->bullets[i];
        if (!bullet->active) {
            bullet->active = true;
            bullet->position = game->player_position;
            bullet->velocity = Vector2Scale(direction, BULLET_SPEED);
            bullet->lifetime = BULLET_LIFETIME;
            sound_play(game, SOUND_PLAYER_SHOOT);
            return;
        }
    }
}

void world_update_player_and_bullets(Game *game, float dt) {
    if (game->player_invulnerable_timer > 0.0f) {
        game->player_invulnerable_timer -= dt;
    }

    Vector2 move = get_move_input();
    game->player_position.x += move.x * game->player_speed * dt;
    game->player_position.y += move.y * game->player_speed * dt;

    game->player_position.x = Clamp(game->player_position.x, game->player_radius, WINDOW_WIDTH - game->player_radius);
    game->player_position.y = Clamp(game->player_position.y, game->player_radius, WINDOW_HEIGHT - game->player_radius);

    if (game->player_fire_timer > 0.0f) {
        game->player_fire_timer -= dt;
    }

    Vector2 aim = get_fire_input();
    if ((aim.x != 0.0f || aim.y != 0.0f) && game->player_fire_timer <= 0.0f) {
        spawn_bullet(game, aim);
        game->player_fire_timer = BULLET_FIRE_RATE;
    }

    for (int i = 0; i < MAX_BULLETS; i++) {
        Bullet *bullet = &game->bullets[i];
        if (!bullet->active) continue;

        bullet->position.x += bullet->velocity.x * dt;
        bullet->position.y += bullet->velocity.y * dt;
        bullet->lifetime -= dt;

        if (bullet->lifetime <= 0.0f ||
            bullet->position.x < -BULLET_RADIUS ||
            bullet->position.x > WINDOW_WIDTH + BULLET_RADIUS ||
            bullet->position.y < -BULLET_RADIUS ||
            bullet->position.y > WINDOW_HEIGHT + BULLET_RADIUS) {
            bullet->active = false;
        }
    }
}

void world_spawn_projectile(Game *game, ProjectileType type, Vector2 position, Vector2 velocity) {
    for (int i = 0; i < MAX_PROJECTILES; i++) {
        EnemyProjectile *projectile = &game->projectiles[i];
        if (!projectile->active) {
            projectile->active = true;
            projectile->type = type;
            projectile->position = position;
            projectile->velocity = velocity;
            projectile->wobble = (float)GetRandomValue(0, 628) / 100.0f;
            if (type == PROJECTILE_CRUISE) {
                projectile->radius = CRUISE_RADIUS;
                projectile->lifetime = CRUISE_LIFETIME;
                sound_play(game, SOUND_BRAIN_MISSILE);
            } else if (type == PROJECTILE_SPARK) {
                projectile->radius = SPARK_RADIUS;
                projectile->lifetime = SPARK_LIFETIME;
                sound_play(game, SOUND_ENEMY_SHOOT);
            } else {
                projectile->radius = SHELL_RADIUS;
                projectile->lifetime = SHELL_LIFETIME;
                sound_play(game, SOUND_ENEMY_SHOOT);
            }
            return;
        }
    }
}

void world_update_projectiles(Game *game, float dt) {
    for (int i = 0; i < MAX_PROJECTILES; i++) {
        EnemyProjectile *projectile = &game->projectiles[i];
        if (!projectile->active) continue;

        projectile->position.x += projectile->velocity.x * dt;
        projectile->position.y += projectile->velocity.y * dt;
        projectile->lifetime -= dt;

        if (projectile->type == PROJECTILE_CRUISE) {
            projectile->wobble += dt * 8.0f;
            Vector2 desired = Vector2Subtract(game->player_position, projectile->position);
            if (desired.x != 0.0f || desired.y != 0.0f) {
                desired = Vector2Normalize(desired);
                Vector2 current = projectile->velocity;
                if (current.x == 0.0f && current.y == 0.0f) {
                    current = desired;
                } else {
                    current = Vector2Normalize(current);
                }

                Vector2 side = (Vector2){ -desired.y, desired.x };
                desired = Vector2Normalize(Vector2Add(desired, Vector2Scale(side, sinf(projectile->wobble) * 0.55f)));
                Vector2 blended = Vector2Normalize(Vector2Add(Vector2Scale(current, 0.88f), Vector2Scale(desired, 0.12f)));
                projectile->velocity = Vector2Scale(blended, CRUISE_SPEED);
            }

            if (projectile->position.x < -projectile->radius ||
                projectile->position.x > WINDOW_WIDTH + projectile->radius ||
                projectile->position.y < -projectile->radius ||
                projectile->position.y > WINDOW_HEIGHT + projectile->radius) {
                projectile->active = false;
                continue;
            }
        } else if (projectile->type == PROJECTILE_SHELL) {
            if (projectile->position.x < projectile->radius || projectile->position.x > WINDOW_WIDTH - projectile->radius) {
                projectile->velocity.x *= -1.0f;
                projectile->position.x = Clamp(projectile->position.x, projectile->radius, WINDOW_WIDTH - projectile->radius);
            }

            if (projectile->position.y < projectile->radius || projectile->position.y > WINDOW_HEIGHT - projectile->radius) {
                projectile->velocity.y *= -1.0f;
                projectile->position.y = Clamp(projectile->position.y, projectile->radius, WINDOW_HEIGHT - projectile->radius);
            }
        } else if (projectile->position.x < -projectile->radius ||
            projectile->position.x > WINDOW_WIDTH + projectile->radius ||
            projectile->position.y < -projectile->radius ||
            projectile->position.y > WINDOW_HEIGHT + projectile->radius) {
            projectile->active = false;
            continue;
        }

        if (projectile->lifetime <= 0.0f) {
            projectile->active = false;
        }
    }
}

void world_draw_projectiles(Game *game) {
    for (int i = 0; i < MAX_PROJECTILES; i++) {
        EnemyProjectile projectile = game->projectiles[i];
        if (!projectile.active) continue;

        if (projectile.type == PROJECTILE_CRUISE) {
            DrawCircleV(projectile.position, projectile.radius + 5.0f, (Color){ 180, 80, 255, 80 });
            DrawCircleV(projectile.position, projectile.radius, VIOLET);
            Vector2 tail = projectile.velocity;
            if (tail.x != 0.0f || tail.y != 0.0f) {
                tail = Vector2Scale(Vector2Normalize(tail), -18.0f);
                DrawLineEx(projectile.position, Vector2Add(projectile.position, tail), 3.0f, Fade(PURPLE, 0.8f));
            }
        } else if (projectile.type == PROJECTILE_SHELL) {
            DrawCircleV(projectile.position, projectile.radius + 4.0f, (Color){ 255, 132, 24, 75 });
            DrawCircleV(projectile.position, projectile.radius, ORANGE);
            DrawCircleLines((int)projectile.position.x, (int)projectile.position.y, projectile.radius + 2.0f, RED);
        } else {
            DrawCircleV(projectile.position, projectile.radius + 4.0f, (Color){ 70, 210, 255, 75 });
            DrawCircleV(projectile.position, projectile.radius, SKYBLUE);
            DrawCircleV(projectile.position, projectile.radius * 0.45f, RAYWHITE);
        }
    }
}

void world_draw_bullets(Game *game) {
    for (int i = 0; i < MAX_BULLETS; i++) {
        Bullet bullet = game->bullets[i];
        if (!bullet.active) continue;

        DrawCircleV(bullet.position, BULLET_RADIUS + 3.0f, (Color){ 77, 214, 255, 70 });
        DrawCircleV(bullet.position, BULLET_RADIUS, RAYWHITE);
    }
}

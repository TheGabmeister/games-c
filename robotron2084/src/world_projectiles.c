#include "world_internal.h"
#include "sounds.h"
#include "raymath.h"

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
            projectile->radius = type == PROJECTILE_SPARK ? SPARK_RADIUS : SHELL_RADIUS;
            projectile->lifetime = type == PROJECTILE_SPARK ? SPARK_LIFETIME : SHELL_LIFETIME;
            sound_play(game, SOUND_ENEMY_SHOOT);
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

        if (projectile->type == PROJECTILE_SHELL) {
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

        if (projectile.type == PROJECTILE_SHELL) {
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

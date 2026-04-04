#include "enemy.h"
#include "collision.h"
#include "draw.h"
#include <math.h>
#include <stdlib.h>

#define MAX_ENEMIES 32
#define DAMAGE_COOLDOWN 1.0f
#define WANDERER_DIR_INTERVAL 2.0f

typedef struct {
    rectangle rect;
    CollisionFilter filter;
    vector2 velocity;
    EnemyType type;
    float speed;
    float timer;
    bool active;
} Enemy;

static Enemy enemies[MAX_ENEMIES];
static float damage_cooldown;

static vector2 random_direction(void)
{
    float angle = (float)(rand() % 360) * 3.14159f / 180.0f;
    return (vector2){ cosf(angle), sinf(angle) };
}

static void clamp_to_window(Enemy *e)
{
    float w = (float)get_window_width();
    float h = (float)get_window_height();

    if (e->rect.x < 0) { e->rect.x = 0; e->velocity.x = -e->velocity.x; }
    if (e->rect.y < 0) { e->rect.y = 0; e->velocity.y = -e->velocity.y; }
    if (e->rect.x + e->rect.w > w) { e->rect.x = w - e->rect.w; e->velocity.x = -e->velocity.x; }
    if (e->rect.y + e->rect.h > h) { e->rect.y = h - e->rect.h; e->velocity.y = -e->velocity.y; }
}

void enemies_init(void)
{
    for (int i = 0; i < MAX_ENEMIES; i++)
        enemies[i].active = false;
    damage_cooldown = 0.0f;
}

void enemy_spawn(EnemyType type, float x, float y)
{
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (enemies[i].active) continue;

        float speed = 0;
        switch (type) {
            case ENEMY_WANDERER: speed = 80.0f;  break;
            case ENEMY_CHASER:   speed = 100.0f; break;
            case ENEMY_BOUNCER:  speed = 150.0f; break;
            default: break;
        }

        vector2 dir = random_direction();

        enemies[i] = (Enemy){
            .rect     = { x, y, 24, 24 },
            .filter   = { COLLISION_LAYER_ENEMY, COLLISION_LAYER_PLAYER | COLLISION_LAYER_PROJECTILE },
            .velocity = { dir.x * speed, dir.y * speed },
            .type     = type,
            .speed    = speed,
            .timer    = 0.0f,
            .active   = true
        };
        return;
    }
}

static void update_wanderer(Enemy *e, float dt)
{
    e->timer += dt;
    if (e->timer >= WANDERER_DIR_INTERVAL) {
        e->timer = 0.0f;
        vector2 dir = random_direction();
        e->velocity.x = dir.x * e->speed;
        e->velocity.y = dir.y * e->speed;
    }

    e->rect.x += e->velocity.x * dt;
    e->rect.y += e->velocity.y * dt;
    clamp_to_window(e);
}

static void update_chaser(Enemy *e, float dt, rectangle player_rect)
{
    float px = player_rect.x + player_rect.w / 2.0f;
    float py = player_rect.y + player_rect.h / 2.0f;
    float ex = e->rect.x + e->rect.w / 2.0f;
    float ey = e->rect.y + e->rect.h / 2.0f;

    float dx = px - ex;
    float dy = py - ey;
    float len = sqrtf(dx * dx + dy * dy);

    if (len > 0.5f) {
        e->rect.x += (dx / len) * e->speed * dt;
        e->rect.y += (dy / len) * e->speed * dt;
    }
}

static void update_bouncer(Enemy *e, float dt)
{
    e->rect.x += e->velocity.x * dt;
    e->rect.y += e->velocity.y * dt;
    clamp_to_window(e);
}

void enemies_update(float dt, rectangle player_rect, CollisionFilter player_filter, GameState *state)
{
    if (damage_cooldown > 0.0f)
        damage_cooldown -= dt;

    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (!enemies[i].active) continue;

        switch (enemies[i].type) {
            case ENEMY_WANDERER: update_wanderer(&enemies[i], dt); break;
            case ENEMY_CHASER:   update_chaser(&enemies[i], dt, player_rect); break;
            case ENEMY_BOUNCER:  update_bouncer(&enemies[i], dt); break;
            default: break;
        }

        if (damage_cooldown <= 0.0f &&
            collision_check(player_rect, player_filter,
                            enemies[i].rect, enemies[i].filter)) {
            state->health -= 1;
            damage_cooldown = DAMAGE_COOLDOWN;
        }
    }
}

void enemies_draw(void)
{
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (!enemies[i].active) continue;

        Enemy *e = &enemies[i];

        switch (e->type) {
            case ENEMY_WANDERER:
                draw_rect_filled(e->rect, COLOR_ORANGE);
                break;
            case ENEMY_CHASER:
                draw_rect_filled(e->rect, COLOR_RED);
                break;
            case ENEMY_BOUNCER:
                draw_rect_filled(e->rect, COLOR_MAGENTA);
                draw_rect(e->rect, COLOR_WHITE);
                break;
            default: break;
        }
    }
}

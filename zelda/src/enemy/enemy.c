#include "enemy.h"
#include "../tilemap.h"
#include "../projectile.h"
#include <stdlib.h>

const EnemyDef enemy_defs[ENEMY_TYPE_COUNT] = {
    [ENEMY_SLIME]         = { 1, 1,  96.0f, false, TILE_SIZE, TILE_SIZE, slime_update,         slime_draw },
    [ENEMY_BAT]           = { 1, 1, 160.0f, true,  TILE_SIZE, TILE_SIZE, bat_update,           bat_draw },
    [ENEMY_SNAKE]         = { 1, 1, 128.0f, false, TILE_SIZE, TILE_SIZE, snake_update,         snake_draw },
    [ENEMY_ROCK_SPITTER]  = { 2, 1,  64.0f, false, TILE_SIZE, TILE_SIZE, rock_spitter_update,  rock_spitter_draw },
    [ENEMY_SPEAR_THROWER] = { 2, 1,  80.0f, false, TILE_SIZE, TILE_SIZE, spear_thrower_update, spear_thrower_draw },
    [ENEMY_DRAGON]        = { 12, 2, 64.0f, false, TILE_SIZE * 2, TILE_SIZE * 2, dragon_update, dragon_draw, dragon_on_death },
};

void enemies_spawn(Enemy enemies[], int *count, const Screen *screen) {
    *count = 0;
    for (int i = 0; i < screen->enemy_spawn_count && *count < MAX_ENEMIES_PER_SCREEN; i++) {
        const EnemySpawn *es = &screen->enemy_spawns[i];
        Enemy *e = &enemies[(*count)++];
        *e = (Enemy){0};
        e->type = (EnemyType)es->type;
        e->pos.x = (float)(es->tile_col * TILE_SIZE);
        e->pos.y = (float)(PLAY_AREA_Y + es->tile_row * TILE_SIZE);
        e->facing = DIR_S;
        e->state = ESTATE_IDLE;
        e->health = enemy_defs[e->type].health;
        e->active = true;
        e->state_timer = 30 + rand() % 60;
        if (e->type == ENEMY_DRAGON) {
            e->velocity.x = (rand() % 2 == 0) ? 64.0f : -64.0f;
            e->ai_timer = 90 + rand() % 60;
        }
    }
}

void enemies_update(Enemy enemies[], int count, Vector2 player_pos, const Screen *screen,
                    Projectile *projectiles, int *projectile_count, float dt) {
    for (int i = 0; i < count; i++) {
        Enemy *e = &enemies[i];
        if (!e->active) continue;
        if (e->invuln_timer > 0) e->invuln_timer--;
        enemy_defs[e->type].update(e, player_pos, screen, projectiles, projectile_count, dt);
    }
}

void enemies_draw(const Enemy enemies[], int count) {
    for (int i = 0; i < count; i++) {
        if (!enemies[i].active) continue;
        enemy_defs[enemies[i].type].draw(&enemies[i]);
    }
}

Rectangle enemy_hitbox(const Enemy *enemy) {
    int w = enemy_defs[enemy->type].hitbox_w;
    int h = enemy_defs[enemy->type].hitbox_h;
    int ox = (TILE_SIZE - w) / 2;
    int oy = (TILE_SIZE - h) / 2;
    return (Rectangle){ enemy->pos.x + ox, enemy->pos.y + oy, (float)w, (float)h };
}

void enemy_take_damage(Enemy *enemy, int damage) {
    enemy->health -= damage;
    if (enemy->health <= 0) {
        enemy->active = false;
        enemy->state = ESTATE_DEAD;
    }
}

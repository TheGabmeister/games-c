#include "enemy.h"
#include "../tilemap.h"
#include "../projectile.h"
#include <stdlib.h>

EnemyDef enemy_defs[ENEMY_TYPE_COUNT];

void enemy_defs_init(void) {
    enemy_defs[ENEMY_SLIME]         = slime_def();
    enemy_defs[ENEMY_BAT]           = bat_def();
    enemy_defs[ENEMY_SNAKE]         = snake_def();
    enemy_defs[ENEMY_ROCK_SPITTER]  = rock_spitter_def();
    enemy_defs[ENEMY_SPEAR_THROWER] = spear_thrower_def();
    enemy_defs[ENEMY_DRAGON]        = dragon_def();
}

void enemies_spawn(Enemy enemies[], int *count, const Screen *screen) {
    *count = 0;
    for (int i = 0; i < screen->enemy_spawn_count && *count < MAX_ENEMIES_PER_SCREEN; i++) {
        const EnemySpawn *es = &screen->enemy_spawns[i];
        Enemy *e = &enemies[(*count)++];
        *e = (Enemy){0};
        e->type = (EnemyType)es->type;
        e->variant = es->variant;
        e->pos.x = (float)(es->tile_col * TILE_SIZE);
        e->pos.y = (float)(PLAY_AREA_Y + es->tile_row * TILE_SIZE);
        e->facing = DIR_S;
        e->state = ESTATE_IDLE;
        e->health = enemy_defs[e->type].health;
        if (e->variant > 0) {
            e->health = e->health * 2;
        }
        e->active = true;
        e->state_timer = 30 + rand() % 60;
        if (enemy_defs[e->type].on_spawn) {
            enemy_defs[e->type].on_spawn(e);
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
    int effective = damage - enemy_defs[enemy->type].defense;
    if (effective < 1) effective = 1;
    enemy->health -= effective;
    if (enemy->health <= 0) {
        enemy->active = false;
        enemy->state = ESTATE_DEAD;
    }
}

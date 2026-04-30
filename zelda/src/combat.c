#include "combat.h"
#include "game.h"
#include "sounds.h"
#include "vfx.h"
#include "world_interact.h"
#include "raymath.h"
#include <math.h>
#include <stdlib.h>

typedef struct DropTable {
    int nothing;
    int rupee;
    int heart;
    int arrow;
    int bomb;
} DropTable;

static const DropTable drop_tables[] = {
    [0] = { 35, 20, 15, 15, 15 },
    [1] = { 20, 30, 20, 15, 15 },
    [2] = { 50, 15, 15, 10, 10 },
};
#define DROP_TABLE_COUNT ((int)(sizeof(drop_tables) / sizeof(drop_tables[0])))

static void on_enemy_death(Game *game, Enemy *e) {
    if (enemy_defs[e->type].on_death) {
        enemy_defs[e->type].on_death(e, game);
        return;
    }
    int group = enemy_defs[e->type].drop_group;
    if (group < 0 || group >= DROP_TABLE_COUNT) group = 0;
    const DropTable *dt = &drop_tables[group];
    int roll = rand() % 100;
    if (roll < dt->nothing) return;
    roll -= dt->nothing;
    PickupType type;
    if (roll < dt->rupee)       type = PICKUP_RUPEE;
    else if (roll < dt->rupee + dt->heart) type = PICKUP_HEART;
    else if (roll < dt->rupee + dt->heart + dt->arrow) type = PICKUP_ARROW;
    else                        type = PICKUP_BOMB;
    pickup_spawn(game->pickups, &game->pickup_count, type, e->pos);
}

static bool try_on_hit(Game *game, Enemy *e, int damage) {
    if (enemy_defs[e->type].on_hit) {
        return enemy_defs[e->type].on_hit(e, game, damage);
    }
    return false;
}

static int player_sword_damage(const Player *p) {
    if (p->inventory.sword_tier >= 3) return SWORD_DAMAGE * 4;
    if (p->inventory.sword_tier >= 2) return SWORD_DAMAGE * 2;
    return SWORD_DAMAGE;
}

void combat_check_pickups(Game *game) {
    Player *p = &game->player;
    Rectangle player_rect = player_hitbox(p);
    for (int i = 0; i < game->pickup_count; i++) {
        Pickup *pk = &game->pickups[i];
        if (!pk->active) continue;
        if (!CheckCollisionRecs(player_rect, pickup_hitbox(pk))) continue;

        switch (pk->type) {
            case PICKUP_RUPEE:
                p->inventory.rupees += pk->value;
                if (p->inventory.rupees > 255) p->inventory.rupees = 255;
                sound_play(SOUND_PICKUP_RUPEE);
                break;
            case PICKUP_HEART:
                p->health += pk->value;
                if (p->health > p->max_health) p->health = p->max_health;
                sound_play(SOUND_PICKUP_HEART);
                break;
            case PICKUP_BOMB:
                p->inventory.bombs += pk->value;
                if (p->inventory.bombs > p->inventory.bomb_capacity)
                    p->inventory.bombs = p->inventory.bomb_capacity;
                sound_play(SOUND_PICKUP_BOMB);
                break;
            case PICKUP_ARROW:
                p->inventory.arrows += pk->value;
                if (p->inventory.arrows > p->inventory.arrow_capacity)
                    p->inventory.arrows = p->inventory.arrow_capacity;
                sound_play(SOUND_PICKUP_BOMB);
                break;
            default: break;
        }
        pk->active = false;
    }
}

void combat_check_bombs(Game *game) {
    for (int pi = 0; pi < game->projectile_count; pi++) {
        Projectile *proj = &game->projectiles[pi];
        if (!proj->active || proj->type != PROJ_BOMB || proj->timer > 0) continue;

        sound_play(SOUND_BOMB_EXPLODE);
        float cx = proj->pos.x + TILE_SIZE / 2.0f;
        float cy = proj->pos.y + TILE_SIZE / 2.0f;
        float r2 = BOMB_BLAST_RADIUS * BOMB_BLAST_RADIUS;

        for (int i = 0; i < game->enemy_count; i++) {
            Enemy *e = &game->enemies[i];
            if (!e->active) continue;
            float dx = (e->pos.x + TILE_SIZE / 2.0f) - cx;
            float dy = (e->pos.y + TILE_SIZE / 2.0f) - cy;
            if (dx * dx + dy * dy <= r2) {
                enemy_take_damage(e, BOMB_DAMAGE);
                if (!e->active) {
                    sound_play(SOUND_ENEMY_DEATH);
                    on_enemy_death(game, e);
                }
            }
        }

        Player *p = &game->player;
        float pdx = (p->pos.x + TILE_SIZE / 2.0f) - cx;
        float pdy = (p->pos.y + TILE_SIZE / 2.0f) - cy;
        if (pdx * pdx + pdy * pdy <= r2) {
            Direction kb_dir;
            if (fabsf(pdx) > fabsf(pdy))
                kb_dir = (pdx > 0) ? DIR_E : DIR_W;
            else
                kb_dir = (pdy > 0) ? DIR_S : DIR_N;
            player_take_damage(p, BOMB_DAMAGE, kb_dir);
            sound_play(SOUND_PLAYER_DAMAGE);
        }

        int tc = (int)(cx / TILE_SIZE);
        int tr = (int)((cy - PLAY_AREA_Y) / TILE_SIZE);
        for (int dr = -2; dr <= 2; dr++) {
            for (int dc = -2; dc <= 2; dc++) {
                int r = tr + dr, c = tc + dc;
                if (r < 0 || r >= SCREEN_TILES_Y || c < 0 || c >= SCREEN_TILES_X) continue;
                if (game->current_screen.tiles[r][c] == TILE_BOMBABLE_WALL) {
                    world_mark_bombable_revealed(game);
                    game->current_screen.tiles[r][c] = TILE_FLOOR;
                }
            }
        }

        vfx_spawn(VFX_EXPLOSION, (Vector2){ cx, cy }, BOMB_BLAST_RADIUS);
        proj->active = false;
    }
}

void combat_check(Game *game) {
    Player *p = &game->player;

    if (p->state == PSTATE_ATTACKING) {
        Rectangle sword = player_sword_hitbox(p);
        if (sword.width > 0) {
            for (int i = 0; i < game->enemy_count; i++) {
                Enemy *e = &game->enemies[i];
                if (!e->active || e->invuln_timer > 0) continue;
                if (CheckCollisionRecs(sword, enemy_hitbox(e))) {
                    e->invuln_timer = SWORD_ACTIVE_FRAMES;
                    sound_play(SOUND_SWORD_HIT);
                    if (try_on_hit(game, e, player_sword_damage(p))) {
                        sound_play(SOUND_ENEMY_DEATH);
                    } else {
                        enemy_take_damage(e, player_sword_damage(p));
                        if (!e->active) {
                            sound_play(SOUND_ENEMY_DEATH);
                            on_enemy_death(game, e);
                        }
                    }
                }
            }
        }
    }

    for (int pi = 0; pi < game->projectile_count; pi++) {
        Projectile *proj = &game->projectiles[pi];
        if (!proj->active || proj->owner != OWNER_PLAYER) continue;
        Rectangle proj_rect = projectile_hitbox(proj);
        for (int i = 0; i < game->enemy_count; i++) {
            Enemy *e = &game->enemies[i];
            if (!e->active || e->invuln_timer > 0) continue;
            if (!CheckCollisionRecs(proj_rect, enemy_hitbox(e))) continue;

            if (try_on_hit(game, e, proj->damage)) {
                sound_play(SOUND_ENEMY_DEATH);
            } else {
                enemy_take_damage(e, proj->damage);
                e->invuln_timer = SWORD_ACTIVE_FRAMES;
                if (!e->active) {
                    sound_play(SOUND_ENEMY_DEATH);
                    on_enemy_death(game, e);
                } else {
                    sound_play(SOUND_SWORD_HIT);
                }
            }

            if (proj->stun_frames > 0 && e->active) {
                e->state = ESTATE_IDLE;
                e->state_timer = proj->stun_frames;
                e->velocity = (Vector2){0, 0};
            }
            if (proj->type != PROJ_BOOMERANG) proj->active = false;
            break;
        }
    }

    if (p->invuln_timer <= 0 && p->state != PSTATE_KNOCKBACK) {
        Rectangle player_rect = player_hitbox(p);
        for (int i = 0; i < game->enemy_count; i++) {
            Enemy *e = &game->enemies[i];
            if (!e->active) continue;
            if (CheckCollisionRecs(player_rect, enemy_hitbox(e))) {
                float dx = p->pos.x - e->pos.x;
                float dy = p->pos.y - e->pos.y;
                Direction kb_dir;
                if (fabsf(dx) > fabsf(dy))
                    kb_dir = (dx > 0) ? DIR_E : DIR_W;
                else
                    kb_dir = (dy > 0) ? DIR_S : DIR_N;
                player_take_damage(p, enemy_defs[e->type].contact_damage, kb_dir);
                sound_play(SOUND_PLAYER_DAMAGE);
                break;
            }
        }
    }

    if (p->invuln_timer <= 0 && p->state != PSTATE_KNOCKBACK) {
        Rectangle player_rect = player_hitbox(p);
        for (int pi = 0; pi < game->projectile_count; pi++) {
            Projectile *proj = &game->projectiles[pi];
            if (!proj->active || proj->owner != OWNER_ENEMY) continue;
            if (!CheckCollisionRecs(player_rect, projectile_hitbox(proj))) continue;

            bool blocked = false;
            if (p->state != PSTATE_ATTACKING && p->inventory.shield_tier >= 1) {
                Direction proj_from = opposite_dir(proj->facing);
                if (proj_from == p->facing) {
                    const ProjectileDef *pdef = &projectile_defs[proj->type];
                    if (pdef->blocked_by_shield_small ||
                        (p->inventory.shield_tier >= 2 && pdef->blocked_by_shield_large)) {
                        blocked = true;
                    }
                }
            }

            if (blocked) {
                proj->active = false;
                sound_play(SOUND_SHIELD_BLOCK);
            } else {
                player_take_damage(p, proj->damage, proj->facing);
                sound_play(SOUND_PLAYER_DAMAGE);
                proj->active = false;
            }
            break;
        }
    }
}

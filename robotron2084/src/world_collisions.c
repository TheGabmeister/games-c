#include "world_internal.h"
#include "sounds.h"
#include "raymath.h"

static void kill_enemy_from_bullet(Game *game, Bullet *bullet, bool *active, Vector2 position,
    int score, Color particle_color, int particle_count, float particle_speed, float particle_radius) {
    world_add_particles(game, position, particle_color, particle_count, particle_speed, particle_radius);
    bullet->active = false;
    *active = false;
    world_add_score(game, score);
    sound_play(game, SOUND_ENEMY_EXPLODE);
}

static bool resolve_bullet_vs_projectiles(Game *game, Bullet *bullet) {
    for (int projectile_index = 0; projectile_index < MAX_PROJECTILES; projectile_index++) {
        EnemyProjectile *projectile = &game->projectiles[projectile_index];
        if (!projectile->active) continue;

        if (world_circles_overlap(bullet->position, BULLET_RADIUS, projectile->position, projectile->radius)) {
            int score = SPARK_SCORE;
            Color color = SKYBLUE;
            if (projectile->type == PROJECTILE_SHELL) {
                score = SHELL_SCORE;
                color = ORANGE;
            } else if (projectile->type == PROJECTILE_CRUISE) {
                score = CRUISE_SCORE;
                color = VIOLET;
            }
            world_add_particles(game, projectile->position, color, 7, 150.0f, 2.5f);
            world_add_score(game, score);
            bullet->active = false;
            projectile->active = false;
            sound_play(game, SOUND_ENEMY_EXPLODE);
            return true;
        }
    }

    return false;
}

static bool resolve_bullet_vs_electrodes(Game *game, Bullet *bullet) {
    for (int electrode_index = 0; electrode_index < MAX_ELECTRODES; electrode_index++) {
        Electrode *electrode = &game->electrodes[electrode_index];
        if (!electrode->active) continue;

        if (world_circles_overlap(bullet->position, BULLET_RADIUS, electrode->position, electrode->radius)) {
            world_add_particles(game, electrode->position, YELLOW, 8, 160.0f, 3.0f);
            bullet->active = false;
            electrode->active = false;
            sound_play(game, SOUND_ENEMY_EXPLODE);
            return true;
        }
    }

    return false;
}

static bool resolve_bullet_vs_hulks(Game *game, Bullet *bullet) {
    for (int hulk_index = 0; hulk_index < MAX_HULKS; hulk_index++) {
        Hulk *hulk = &game->hulks[hulk_index];
        if (!hulk->active) continue;

        if (world_circles_overlap(bullet->position, BULLET_RADIUS, hulk->position, hulk->radius)) {
            Vector2 shove = Vector2Subtract(hulk->position, bullet->position);
            if (shove.x != 0.0f || shove.y != 0.0f) {
                shove = Vector2Scale(Vector2Normalize(shove), HULK_KNOCKBACK);
                hulk->position = Vector2Add(hulk->position, shove);
            }
            hulk->position.x = Clamp(hulk->position.x, hulk->radius, WINDOW_WIDTH - hulk->radius);
            hulk->position.y = Clamp(hulk->position.y, hulk->radius, WINDOW_HEIGHT - hulk->radius);
            hulk->stun_timer = HULK_BULLET_STUN_TIME;
            bullet->active = false;
            world_add_particles(game, hulk->position, LIME, 5, 105.0f, 2.5f);
            sound_play(game, SOUND_HULK_HIT);
            return true;
        }
    }

    return false;
}

static bool resolve_bullet_vs_spawners_and_shooters(Game *game, Bullet *bullet) {
    for (int spheroid_index = 0; spheroid_index < MAX_SPHEROIDS; spheroid_index++) {
        Spheroid *spheroid = &game->spheroids[spheroid_index];
        if (!spheroid->active) continue;

        if (world_circles_overlap(bullet->position, BULLET_RADIUS, spheroid->position, spheroid->radius)) {
            kill_enemy_from_bullet(game, bullet, &spheroid->active, spheroid->position,
                SPHEROID_SCORE, BLUE, 12, 175.0f, 3.0f);
            return true;
        }
    }

    for (int enforcer_index = 0; enforcer_index < MAX_ENFORCERS; enforcer_index++) {
        Enforcer *enforcer = &game->enforcers[enforcer_index];
        if (!enforcer->active) continue;

        if (world_circles_overlap(bullet->position, BULLET_RADIUS, enforcer->position, enforcer->radius)) {
            kill_enemy_from_bullet(game, bullet, &enforcer->active, enforcer->position,
                ENFORCER_SCORE, SKYBLUE, 10, 165.0f, 3.0f);
            return true;
        }
    }

    for (int quark_index = 0; quark_index < MAX_QUARKS; quark_index++) {
        Quark *quark = &game->quarks[quark_index];
        if (!quark->active) continue;

        if (world_circles_overlap(bullet->position, BULLET_RADIUS, quark->position, quark->radius)) {
            kill_enemy_from_bullet(game, bullet, &quark->active, quark->position,
                QUARK_SCORE, RAYWHITE, 12, 175.0f, 3.0f);
            return true;
        }
    }

    for (int tank_index = 0; tank_index < MAX_TANKS; tank_index++) {
        Tank *tank = &game->tanks[tank_index];
        if (!tank->active) continue;

        if (world_circles_overlap(bullet->position, BULLET_RADIUS, tank->position, tank->radius)) {
            kill_enemy_from_bullet(game, bullet, &tank->active, tank->position,
                TANK_SCORE, RED, 10, 165.0f, 3.0f);
            return true;
        }
    }

    return false;
}

static bool resolve_bullet_vs_brains_and_progs(Game *game, Bullet *bullet) {
    for (int brain_index = 0; brain_index < MAX_BRAINS; brain_index++) {
        Brain *brain = &game->brains[brain_index];
        if (!brain->active) continue;

        if (world_circles_overlap(bullet->position, BULLET_RADIUS, brain->position, brain->radius)) {
            kill_enemy_from_bullet(game, bullet, &brain->active, brain->position,
                BRAIN_SCORE, VIOLET, 12, 175.0f, 3.0f);
            return true;
        }
    }

    for (int prog_index = 0; prog_index < MAX_PROGS; prog_index++) {
        Prog *prog = &game->progs[prog_index];
        if (!prog->active) continue;

        if (world_circles_overlap(bullet->position, BULLET_RADIUS, prog->position, prog->radius)) {
            kill_enemy_from_bullet(game, bullet, &prog->active, prog->position,
                PROG_SCORE, PURPLE, 9, 165.0f, 2.6f);
            return true;
        }
    }

    return false;
}

static bool resolve_bullet_vs_grunts(Game *game, Bullet *bullet) {
    for (int grunt_index = 0; grunt_index < MAX_GRUNTS; grunt_index++) {
        Grunt *grunt = &game->grunts[grunt_index];
        if (!grunt->active) continue;

        if (world_circles_overlap(bullet->position, BULLET_RADIUS, grunt->position, grunt->radius)) {
            kill_enemy_from_bullet(game, bullet, &grunt->active, grunt->position,
                GRUNT_SCORE, RED, 10, 180.0f, 3.0f);
            return true;
        }
    }

    return false;
}

void world_resolve_bullet_collisions(Game *game) {
    for (int bullet_index = 0; bullet_index < MAX_BULLETS; bullet_index++) {
        Bullet *bullet = &game->bullets[bullet_index];
        if (!bullet->active) continue;

        if (resolve_bullet_vs_projectiles(game, bullet)) continue;
        if (resolve_bullet_vs_electrodes(game, bullet)) continue;
        if (resolve_bullet_vs_hulks(game, bullet)) continue;
        if (resolve_bullet_vs_spawners_and_shooters(game, bullet)) continue;
        if (resolve_bullet_vs_brains_and_progs(game, bullet)) continue;
        if (resolve_bullet_vs_grunts(game, bullet)) continue;
    }
}

void world_resolve_brain_human_collisions(Game *game) {
    for (int brain_index = 0; brain_index < MAX_BRAINS; brain_index++) {
        Brain *brain = &game->brains[brain_index];
        if (!brain->active) continue;

        for (int human_index = 0; human_index < MAX_HUMANS; human_index++) {
            Human *human = &game->humans[human_index];
            if (!human->active) continue;

            if (world_circles_overlap(brain->position, brain->radius, human->position, human->radius)) {
                Vector2 position = human->position;
                human->active = false;
                world_spawn_prog(game, position);
                world_add_particles(game, position, VIOLET, 8, 135.0f, 2.4f);
                break;
            }
        }
    }
}

void world_resolve_human_collisions(Game *game) {
    for (int i = 0; i < MAX_HUMANS; i++) {
        Human *human = &game->humans[i];
        if (!human->active) continue;

        if (world_circles_overlap(game->player_position, game->player_radius, human->position, human->radius)) {
            int rescue_score = world_next_human_rescue_score(game);
            human->active = false;
            game->humans_rescued_this_wave++;
            world_add_score(game, rescue_score);
            world_add_float_text(game, human->position, rescue_score, GOLD);
            world_add_particles(game, human->position, GOLD, 8, 120.0f, 2.5f);
            sound_play(game, SOUND_HUMAN_RESCUE);
        }
    }

    for (int hulk_index = 0; hulk_index < MAX_HULKS; hulk_index++) {
        Hulk *hulk = &game->hulks[hulk_index];
        if (!hulk->active) continue;

        for (int human_index = 0; human_index < MAX_HUMANS; human_index++) {
            Human *human = &game->humans[human_index];
            if (!human->active) continue;

            if (world_circles_overlap(hulk->position, hulk->radius, human->position, human->radius)) {
                world_add_particles(game, human->position, ORANGE, 6, 110.0f, 2.0f);
                human->active = false;
            }
        }
    }
}

void world_resolve_grunt_electrode_collisions(Game *game) {
    for (int grunt_index = 0; grunt_index < MAX_GRUNTS; grunt_index++) {
        Grunt *grunt = &game->grunts[grunt_index];
        if (!grunt->active) continue;

        for (int electrode_index = 0; electrode_index < MAX_ELECTRODES; electrode_index++) {
            Electrode *electrode = &game->electrodes[electrode_index];
            if (!electrode->active) continue;

            if (world_circles_overlap(grunt->position, grunt->radius, electrode->position, electrode->radius)) {
                world_add_particles(game, grunt->position, RED, 7, 150.0f, 2.5f);
                grunt->active = false;
                break;
            }
        }
    }
}

static void handle_player_death(Game *game) {
    world_add_particles(game, game->player_position, SKYBLUE, 18, 230.0f, 3.0f);
    game->screen_shake = SCREEN_SHAKE_DURATION;
    game->screen_flash = SCREEN_FLASH_DURATION;
    sound_play(game, SOUND_PLAYER_DIE);
    game->lives--;
    if (game->lives <= 0) {
        game->mode = GAME_MODE_GAME_OVER;
        memset(game->bullets, 0, sizeof(game->bullets));
        memset(game->projectiles, 0, sizeof(game->projectiles));
    } else {
        game->mode = GAME_MODE_PLAYER_DEAD;
        game->mode_timer = PLAYER_DEATH_PAUSE;
        game->humans_rescued_this_wave = 0;
        memset(game->bullets, 0, sizeof(game->bullets));
        memset(game->projectiles, 0, sizeof(game->projectiles));
    }
}

static bool player_overlaps_circle(Game *game, Vector2 position, float radius) {
    return world_circles_overlap(game->player_position, game->player_radius, position, radius);
}

static bool player_overlaps_hulks(Game *game) {
    for (int i = 0; i < MAX_HULKS; i++) {
        Hulk *hulk = &game->hulks[i];
        if (hulk->active && player_overlaps_circle(game, hulk->position, hulk->radius)) {
            return true;
        }
    }

    return false;
}

static bool player_overlaps_spawners_and_shooters(Game *game) {
    for (int i = 0; i < MAX_SPHEROIDS; i++) {
        Spheroid *spheroid = &game->spheroids[i];
        if (spheroid->active && player_overlaps_circle(game, spheroid->position, spheroid->radius)) {
            return true;
        }
    }

    for (int i = 0; i < MAX_ENFORCERS; i++) {
        Enforcer *enforcer = &game->enforcers[i];
        if (enforcer->active && player_overlaps_circle(game, enforcer->position, enforcer->radius)) {
            return true;
        }
    }

    for (int i = 0; i < MAX_QUARKS; i++) {
        Quark *quark = &game->quarks[i];
        if (quark->active && player_overlaps_circle(game, quark->position, quark->radius)) {
            return true;
        }
    }

    for (int i = 0; i < MAX_TANKS; i++) {
        Tank *tank = &game->tanks[i];
        if (tank->active && player_overlaps_circle(game, tank->position, tank->radius)) {
            return true;
        }
    }

    return false;
}

static bool player_overlaps_brains_and_progs(Game *game) {
    for (int i = 0; i < MAX_BRAINS; i++) {
        Brain *brain = &game->brains[i];
        if (brain->active && player_overlaps_circle(game, brain->position, brain->radius)) {
            return true;
        }
    }

    for (int i = 0; i < MAX_PROGS; i++) {
        Prog *prog = &game->progs[i];
        if (prog->active && player_overlaps_circle(game, prog->position, prog->radius)) {
            return true;
        }
    }

    return false;
}

static bool player_overlaps_grunts(Game *game) {
    for (int i = 0; i < MAX_GRUNTS; i++) {
        Grunt *grunt = &game->grunts[i];
        if (grunt->active && player_overlaps_circle(game, grunt->position, grunt->radius)) {
            return true;
        }
    }

    return false;
}

void world_resolve_player_death_collisions(Game *game) {
    if (game->player_invulnerable_timer > 0.0f) {
        return;
    }

    for (int i = 0; i < MAX_PROJECTILES; i++) {
        EnemyProjectile *projectile = &game->projectiles[i];
        if (projectile->active && world_circles_overlap(game->player_position, game->player_radius, projectile->position, projectile->radius)) {
            handle_player_death(game);
            return;
        }
    }

    for (int i = 0; i < MAX_ELECTRODES; i++) {
        Electrode *electrode = &game->electrodes[i];
        if (electrode->active && world_circles_overlap(game->player_position, game->player_radius, electrode->position, electrode->radius)) {
            handle_player_death(game);
            return;
        }
    }

    if (player_overlaps_hulks(game) ||
        player_overlaps_spawners_and_shooters(game) ||
        player_overlaps_brains_and_progs(game) ||
        player_overlaps_grunts(game)) {
        handle_player_death(game);
    }
}

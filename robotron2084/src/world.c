#include "world_internal.h"
#include "sounds.h"
#include "raymath.h"

int world_count_active_grunts(Game *game) {
    int count = 0;
    for (int i = 0; i < MAX_GRUNTS; i++) {
        if (game->grunts[i].active) count++;
    }
    return count;
}

int world_count_active_humans(Game *game) {
    int count = 0;
    for (int i = 0; i < MAX_HUMANS; i++) {
        if (game->humans[i].active) count++;
    }
    return count;
}

int world_count_active_hulks(Game *game) {
    int count = 0;
    for (int i = 0; i < MAX_HULKS; i++) {
        if (game->hulks[i].active) count++;
    }
    return count;
}

int world_count_active_spheroids(Game *game) {
    int count = 0;
    for (int i = 0; i < MAX_SPHEROIDS; i++) {
        if (game->spheroids[i].active) count++;
    }
    return count;
}

int world_count_active_enforcers(Game *game) {
    int count = 0;
    for (int i = 0; i < MAX_ENFORCERS; i++) {
        if (game->enforcers[i].active) count++;
    }
    return count;
}

int world_count_active_quarks(Game *game) {
    int count = 0;
    for (int i = 0; i < MAX_QUARKS; i++) {
        if (game->quarks[i].active) count++;
    }
    return count;
}

int world_count_active_tanks(Game *game) {
    int count = 0;
    for (int i = 0; i < MAX_TANKS; i++) {
        if (game->tanks[i].active) count++;
    }
    return count;
}

int world_count_active_brains(Game *game) {
    int count = 0;
    for (int i = 0; i < MAX_BRAINS; i++) {
        if (game->brains[i].active) count++;
    }
    return count;
}

int world_count_active_progs(Game *game) {
    int count = 0;
    for (int i = 0; i < MAX_PROGS; i++) {
        if (game->progs[i].active) count++;
    }
    return count;
}

int world_count_active_projectiles(Game *game) {
    int count = 0;
    for (int i = 0; i < MAX_PROJECTILES; i++) {
        if (game->projectiles[i].active) count++;
    }
    return count;
}

int world_count_active_electrodes(Game *game) {
    int count = 0;
    for (int i = 0; i < MAX_ELECTRODES; i++) {
        if (game->electrodes[i].active) count++;
    }
    return count;
}

void world_reset_player(Game *game) {
    game->player_position = (Vector2){ PLAYER_START_X, PLAYER_START_Y };
    game->player_fire_timer = 0.0f;
    game->player_invulnerable_timer = RESPAWN_INVULN_TIME;
    game->humans_rescued_this_wave = 0;
    memset(game->bullets, 0, sizeof(game->bullets));
    memset(game->projectiles, 0, sizeof(game->projectiles));
}

static void clear_wave_entities(Game *game) {
    memset(game->bullets, 0, sizeof(game->bullets));
    memset(game->grunts, 0, sizeof(game->grunts));
    memset(game->hulks, 0, sizeof(game->hulks));
    memset(game->spheroids, 0, sizeof(game->spheroids));
    memset(game->enforcers, 0, sizeof(game->enforcers));
    memset(game->quarks, 0, sizeof(game->quarks));
    memset(game->tanks, 0, sizeof(game->tanks));
    memset(game->brains, 0, sizeof(game->brains));
    memset(game->progs, 0, sizeof(game->progs));
    memset(game->projectiles, 0, sizeof(game->projectiles));
    memset(game->humans, 0, sizeof(game->humans));
    memset(game->electrodes, 0, sizeof(game->electrodes));
    memset(game->float_text, 0, sizeof(game->float_text));
    memset(game->particles, 0, sizeof(game->particles));
}

static Vector2 random_edge_position(float margin) {
    int side = GetRandomValue(0, 3);
    Vector2 position = { 0.0f, 0.0f };

    if (side == 0) {
        position.x = (float)GetRandomValue((int)margin, WINDOW_WIDTH - (int)margin);
        position.y = margin;
    } else if (side == 1) {
        position.x = (float)GetRandomValue((int)margin, WINDOW_WIDTH - (int)margin);
        position.y = WINDOW_HEIGHT - margin;
    } else if (side == 2) {
        position.x = margin;
        position.y = (float)GetRandomValue((int)margin, WINDOW_HEIGHT - (int)margin);
    } else {
        position.x = WINDOW_WIDTH - margin;
        position.y = (float)GetRandomValue((int)margin, WINDOW_HEIGHT - (int)margin);
    }

    return position;
}

static Vector2 random_arena_position(float margin) {
    return (Vector2){
        (float)GetRandomValue((int)margin, WINDOW_WIDTH - (int)margin),
        (float)GetRandomValue((int)margin, WINDOW_HEIGHT - (int)margin)
    };
}

void world_spawn_wave(Game *game) {
    clear_wave_entities(game);
    world_reset_player(game);

    bool brain_wave = (game->wave % 5) == 0;

    if (brain_wave) {
        int brain_count = 6 + game->wave / 3;
        if (brain_count > MAX_BRAINS) brain_count = MAX_BRAINS;
        for (int i = 0; i < brain_count; i++) {
            world_spawn_brain(game, random_edge_position(76.0f));
        }

        int human_count = 10 + game->wave / 2;
        if (human_count > MAX_HUMANS) human_count = MAX_HUMANS;
        for (int i = 0; i < human_count; i++) {
            world_spawn_human(game, random_arena_position(80.0f), i % 3);
        }

        int electrode_count = 8 + game->wave / 10;
        if (electrode_count > WAVE_ELECTRODE_MAX) electrode_count = WAVE_ELECTRODE_MAX;
        for (int i = 0; i < electrode_count; i++) {
            Vector2 position = random_arena_position(70.0f);
            if (Vector2Distance(position, game->player_position) > 135.0f) {
                world_spawn_electrode(game, position);
            }
        }

        return;
    }

    int grunt_count = WAVE_START_GRUNTS + (game->wave - 1) * WAVE_GRUNT_STEP;
    if (grunt_count > MAX_GRUNTS) grunt_count = MAX_GRUNTS;

    for (int i = 0; i < grunt_count; i++) {
        world_spawn_grunt(game, random_edge_position(42.0f));
    }

    if (game->wave >= 2) {
        int spheroid_count = 1 + game->wave / 2;
        if (spheroid_count > 5) spheroid_count = 5;
        for (int i = 0; i < spheroid_count; i++) {
            world_spawn_spheroid(game, random_edge_position(72.0f));
        }
    }

    if (game->wave >= 7) {
        int quark_count = 1 + (game->wave - 7) / 3;
        if (quark_count > 3) quark_count = 3;
        for (int i = 0; i < quark_count; i++) {
            world_spawn_quark(game, random_edge_position(84.0f));
        }

        int tank_count = 2 + (game->wave - 7) / 2;
        if (tank_count > 6) tank_count = 6;
        for (int i = 0; i < tank_count; i++) {
            world_spawn_tank(game, random_edge_position(90.0f));
        }
    }

    int human_count = WAVE_START_HUMANS + game->wave / 2;
    if (human_count > WAVE_HUMAN_MAX) human_count = WAVE_HUMAN_MAX;

    for (int i = 0; i < human_count; i++) {
        world_spawn_human(game, random_arena_position(80.0f), i % 3);
    }

    int hulk_count = 1 + game->wave / 3;
    if (hulk_count > MAX_HULKS) hulk_count = MAX_HULKS;

    for (int i = 0; i < hulk_count; i++) {
        world_spawn_hulk(game, random_edge_position(70.0f));
    }

    int electrode_count = WAVE_START_ELECTRODES + game->wave;
    if (electrode_count > WAVE_ELECTRODE_MAX) electrode_count = WAVE_ELECTRODE_MAX;

    for (int i = 0; i < electrode_count; i++) {
        Vector2 position = random_arena_position(70.0f);
        if (Vector2Distance(position, game->player_position) > 135.0f) {
            world_spawn_electrode(game, position);
        }
    }
}

int world_next_human_rescue_score(Game *game) {
    int rescue_index = game->humans_rescued_this_wave + 1;
    if (rescue_index >= 5) {
        return 5000;
    }
    return rescue_index * 1000;
}

void world_update_playing(Game *game, float dt) {
    world_update_player_and_bullets(game, dt);
    world_update_humans(game, dt);
    world_update_grunts_and_hulks(game, dt);
    world_update_spawners_and_shooters(game, dt);
    world_update_brains_and_progs(game, dt);
    world_update_projectiles(game, dt);
    world_update_effects(game, dt);

    world_resolve_bullet_collisions(game);
    world_resolve_human_collisions(game);
    world_resolve_grunt_electrode_collisions(game);

    if (world_count_active_grunts(game) == 0 &&
        world_count_active_spheroids(game) == 0 &&
        world_count_active_enforcers(game) == 0 &&
        world_count_active_quarks(game) == 0 &&
        world_count_active_tanks(game) == 0 &&
        world_count_active_brains(game) == 0 &&
        world_count_active_progs(game) == 0) {
        sound_play(game, SOUND_WAVE_CLEAR);
        game->screen_flash = 0.12f;
        game->wave++;
        world_spawn_wave(game);
        return;
    }

    world_resolve_player_death_collisions(game);
}

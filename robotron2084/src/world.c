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

typedef struct WavePlan {
    int grunts;
    int hulks;
    int spheroids;
    int enforcers;
    int quarks;
    int tanks;
    int brains;
    int humans;
    int electrodes;
} WavePlan;

static int clamp_int(int value, int min, int max) {
    if (value < min) return min;
    if (value > max) return max;
    return value;
}

static int wave_cycle(int wave) {
    int cycle = ((wave - 1) % 255) + 1;
    return cycle > 0 ? cycle : 1;
}

static WavePlan build_wave_plan(int wave) {
    int w = wave_cycle(wave);
    int decade = (w - 1) / 10;

    static const WavePlan templates[10] = {
        { 18, 2, 0, 0, 0, 0, 0, 5, 10 },
        { 24, 2, 2, 0, 0, 0, 0, 5, 12 },
        { 32, 3, 3, 0, 0, 0, 0, 6, 12 },
        { 38, 3, 3, 0, 0, 0, 0, 6, 14 },
        { 0, 0, 0, 0, 0, 0, 8, 12, 8 },
        { 42, 4, 4, 0, 0, 0, 0, 6, 12 },
        { 16, 0, 0, 0, 2, 4, 0, 4, 10 },
        { 44, 4, 4, 0, 0, 0, 0, 5, 14 },
        { 70, 2, 0, 0, 0, 0, 0, 4, 8 },
        { 0, 0, 0, 0, 0, 0, 10, 14, 8 },
    };

    if (w <= 10) {
        return templates[w - 1];
    }

    if ((w % 5) == 0) {
        WavePlan plan = { 0 };
        plan.brains = clamp_int(8 + decade, 8, MAX_BRAINS);
        plan.humans = clamp_int(12 + decade, 12, 22);
        plan.electrodes = clamp_int(8 + decade / 2, 8, WAVE_ELECTRODE_MAX);
        return plan;
    }

    if ((w % 20) == 14) {
        WavePlan plan = { 0 };
        plan.grunts = clamp_int(18 + decade * 3, 18, 46);
        plan.hulks = clamp_int(8 + decade, 8, MAX_HULKS);
        plan.humans = clamp_int(3 + decade / 4, 3, 6);
        plan.electrodes = clamp_int(10 + decade / 2, 10, WAVE_ELECTRODE_MAX);
        return plan;
    }

    if ((w % 10) == 9) {
        WavePlan plan = { 0 };
        plan.grunts = clamp_int(70 + decade * 3, 70, MAX_GRUNTS);
        plan.hulks = clamp_int(2 + decade / 4, 2, 6);
        plan.humans = 4;
        plan.electrodes = clamp_int(8 + decade / 2, 8, 16);
        return plan;
    }

    if (w >= 24 && (w % 20) == 4) {
        WavePlan plan = { 0 };
        plan.grunts = clamp_int(22 + decade * 2, 22, 50);
        plan.spheroids = clamp_int(3 + decade / 5, 3, MAX_SPHEROIDS);
        plan.enforcers = clamp_int(3 + decade / 4, 3, 10);
        plan.quarks = clamp_int(2 + decade / 5, 2, MAX_QUARKS);
        plan.tanks = clamp_int(4 + decade / 3, 4, 12);
        plan.humans = 4;
        plan.electrodes = clamp_int(12 + decade / 2, 12, WAVE_ELECTRODE_MAX);
        return plan;
    }

    if (w >= 7 && ((w - 7) % 5) == 0) {
        WavePlan plan = { 0 };
        plan.grunts = clamp_int(16 + decade * 3, 16, 54);
        plan.quarks = clamp_int(2 + decade / 4, 2, MAX_QUARKS);
        plan.tanks = clamp_int(4 + decade / 3, 4, 12);
        plan.humans = 4;
        plan.electrodes = clamp_int(10 + decade / 2, 10, WAVE_ELECTRODE_MAX);
        return plan;
    }

    WavePlan plan = { 0 };
    plan.grunts = clamp_int(34 + decade * 4, 34, 72);
    plan.hulks = clamp_int(3 + decade / 3, 3, 9);
    plan.spheroids = clamp_int(3 + decade / 4, 3, 7);
    if (w >= 28) {
        plan.quarks = clamp_int(1 + decade / 6, 1, 4);
        plan.tanks = clamp_int(1 + decade / 5, 1, 6);
    }
    plan.humans = clamp_int(5 + decade / 3, 5, WAVE_HUMAN_MAX);
    plan.electrodes = clamp_int(12 + decade / 2, 12, WAVE_ELECTRODE_MAX);
    return plan;
}

void world_spawn_wave(Game *game) {
    clear_wave_entities(game);
    world_reset_player(game);

    WavePlan plan = build_wave_plan(game->wave);

    for (int i = 0; i < plan.grunts; i++) {
        world_spawn_grunt(game, random_edge_position(42.0f));
    }

    for (int i = 0; i < plan.spheroids; i++) {
        world_spawn_spheroid(game, random_edge_position(72.0f));
    }

    for (int i = 0; i < plan.enforcers; i++) {
        world_spawn_enforcer(game, random_edge_position(84.0f));
    }

    for (int i = 0; i < plan.quarks; i++) {
        world_spawn_quark(game, random_edge_position(84.0f));
    }

    for (int i = 0; i < plan.tanks; i++) {
        world_spawn_tank(game, random_edge_position(90.0f));
    }

    for (int i = 0; i < plan.brains; i++) {
        world_spawn_brain(game, random_edge_position(76.0f));
    }

    for (int i = 0; i < plan.hulks; i++) {
        world_spawn_hulk(game, random_edge_position(70.0f));
    }

    for (int i = 0; i < plan.humans; i++) {
        world_spawn_human(game, random_arena_position(80.0f), i % 3);
    }

    for (int i = 0; i < plan.electrodes; i++) {
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
        if (game->wave > 255) {
            game->wave = 1;
        }
        world_spawn_wave(game);
        game->mode = GAME_MODE_WAVE_INTRO;
        game->mode_timer = WAVE_INTRO_TIME;
        return;
    }

    world_resolve_player_death_collisions(game);
}

#include "world.h"
#include "sounds.h"
#include "textures.h"
#include "raymath.h"
#include <math.h>

static bool circles_overlap(Vector2 a, float ar, Vector2 b, float br) {
    float dx = a.x - b.x;
    float dy = a.y - b.y;
    float radius = ar + br;
    return dx * dx + dy * dy <= radius * radius;
}

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

static void add_float_text(Game *game, Vector2 position, int value, Color color) {
    for (int i = 0; i < MAX_FLOAT_TEXT; i++) {
        FloatText *text = &game->float_text[i];
        if (!text->active) {
            text->active = true;
            text->position = position;
            text->velocity = (Vector2){ 0.0f, -42.0f };
            text->lifetime = FLOAT_TEXT_LIFETIME;
            text->value = value;
            text->color = color;
            return;
        }
    }
}

static void add_particles(Game *game, Vector2 position, Color color, int count, float speed, float radius) {
    for (int n = 0; n < count; n++) {
        for (int i = 0; i < MAX_PARTICLES; i++) {
            Particle *particle = &game->particles[i];
            if (!particle->active) {
                float angle = (float)GetRandomValue(0, 628) / 100.0f;
                float force = speed * (float)GetRandomValue(45, 100) / 100.0f;
                particle->active = true;
                particle->position = position;
                particle->velocity = (Vector2){ cosf(angle) * force, sinf(angle) * force };
                particle->radius = radius * (float)GetRandomValue(60, 120) / 100.0f;
                particle->lifetime = (float)GetRandomValue(25, 55) / 100.0f;
                particle->max_lifetime = particle->lifetime;
                particle->color = color;
                break;
            }
        }
    }
}

static void add_score(Game *game, int value) {
    game->score += value;
    if (game->score > game->high_score) {
        game->high_score = game->score;
    }

    while (game->score >= game->next_extra_life_score) {
        game->lives++;
        game->next_extra_life_score += EXTRA_LIFE_SCORE;
        add_float_text(game, (Vector2){ game->player_position.x, game->player_position.y - 28.0f }, 0, GREEN);
        sound_play(game, SOUND_EXTRA_LIFE);
    }
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

static Vector2 random_human_velocity(void) {
    Vector2 direction = {
        (float)GetRandomValue(-100, 100) / 100.0f,
        (float)GetRandomValue(-100, 100) / 100.0f
    };

    if (direction.x == 0.0f && direction.y == 0.0f) {
        direction.x = 1.0f;
    }

    return Vector2Scale(Vector2Normalize(direction), HUMAN_SPEED);
}

static Vector2 random_direction(void) {
    Vector2 direction = {
        (float)GetRandomValue(-100, 100) / 100.0f,
        (float)GetRandomValue(-100, 100) / 100.0f
    };

    if (direction.x == 0.0f && direction.y == 0.0f) {
        direction.x = 1.0f;
    }

    return Vector2Normalize(direction);
}

static Vector2 edge_drift_velocity(Vector2 position, float speed) {
    Vector2 target = {
        GetRandomValue(0, 1) == 0 ? 56.0f : WINDOW_WIDTH - 56.0f,
        GetRandomValue(0, 1) == 0 ? 56.0f : WINDOW_HEIGHT - 56.0f
    };
    Vector2 to_target = Vector2Subtract(target, position);

    if (to_target.x == 0.0f && to_target.y == 0.0f) {
        return Vector2Scale(random_direction(), speed);
    }

    return Vector2Scale(Vector2Normalize(to_target), speed);
}

static void spawn_grunt(Game *game, Vector2 position) {
    for (int i = 0; i < MAX_GRUNTS; i++) {
        Grunt *grunt = &game->grunts[i];
        if (!grunt->active) {
            grunt->active = true;
            grunt->position = position;
            grunt->speed = GRUNT_SPEED + (float)(game->wave - 1) * 8.0f;
            grunt->radius = GRUNT_RADIUS;
            return;
        }
    }
}

static void spawn_hulk(Game *game, Vector2 position) {
    for (int i = 0; i < MAX_HULKS; i++) {
        Hulk *hulk = &game->hulks[i];
        if (!hulk->active) {
            hulk->active = true;
            hulk->position = position;
            hulk->speed = HULK_SPEED + (float)(game->wave - 1) * 3.0f;
            hulk->radius = HULK_RADIUS;
            hulk->stun_timer = 0.0f;
            return;
        }
    }
}

static void spawn_spheroid(Game *game, Vector2 position) {
    for (int i = 0; i < MAX_SPHEROIDS; i++) {
        Spheroid *spheroid = &game->spheroids[i];
        if (!spheroid->active) {
            spheroid->active = true;
            spheroid->position = position;
            spheroid->speed = SPHEROID_SPEED + (float)(game->wave - 1) * 1.5f;
            spheroid->radius = SPHEROID_RADIUS;
            spheroid->velocity = edge_drift_velocity(position, spheroid->speed);
            spheroid->spawn_timer = (float)GetRandomValue(280, 430) / 100.0f;
            spheroid->retarget_timer = (float)GetRandomValue(120, 240) / 100.0f;
            return;
        }
    }
}

static void spawn_enforcer(Game *game, Vector2 position) {
    for (int i = 0; i < MAX_ENFORCERS; i++) {
        Enforcer *enforcer = &game->enforcers[i];
        if (!enforcer->active) {
            enforcer->active = true;
            enforcer->position = position;
            enforcer->speed = ENFORCER_SPEED + (float)(game->wave - 1) * 2.0f;
            enforcer->radius = ENFORCER_RADIUS;
            enforcer->velocity = edge_drift_velocity(position, enforcer->speed);
            enforcer->shoot_timer = (float)GetRandomValue(90, 170) / 100.0f;
            enforcer->retarget_timer = (float)GetRandomValue(80, 180) / 100.0f;
            return;
        }
    }
}

static void spawn_quark(Game *game, Vector2 position) {
    for (int i = 0; i < MAX_QUARKS; i++) {
        Quark *quark = &game->quarks[i];
        if (!quark->active) {
            quark->active = true;
            quark->position = position;
            quark->speed = QUARK_SPEED + (float)(game->wave - 1) * 1.2f;
            quark->radius = QUARK_RADIUS;
            quark->velocity = edge_drift_velocity(position, quark->speed);
            quark->spawn_timer = (float)GetRandomValue(360, 520) / 100.0f;
            quark->retarget_timer = (float)GetRandomValue(120, 260) / 100.0f;
            return;
        }
    }
}

static void spawn_tank(Game *game, Vector2 position) {
    for (int i = 0; i < MAX_TANKS; i++) {
        Tank *tank = &game->tanks[i];
        if (!tank->active) {
            tank->active = true;
            tank->position = position;
            tank->speed = TANK_SPEED + (float)(game->wave - 1) * 1.0f;
            tank->radius = TANK_RADIUS;
            tank->velocity = Vector2Scale(random_direction(), tank->speed);
            tank->shoot_timer = (float)GetRandomValue(120, 220) / 100.0f;
            tank->retarget_timer = (float)GetRandomValue(120, 250) / 100.0f;
            return;
        }
    }
}

static void spawn_projectile(Game *game, ProjectileType type, Vector2 position, Vector2 velocity) {
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

static void spawn_human(Game *game, Vector2 position, int type) {
    for (int i = 0; i < MAX_HUMANS; i++) {
        Human *human = &game->humans[i];
        if (!human->active) {
            human->active = true;
            human->type = type;
            human->position = position;
            human->velocity = random_human_velocity();
            human->radius = HUMAN_RADIUS;
            human->retarget_timer = (float)GetRandomValue(80, 180) / 100.0f;
            return;
        }
    }
}

static void spawn_electrode(Game *game, Vector2 position) {
    for (int i = 0; i < MAX_ELECTRODES; i++) {
        Electrode *electrode = &game->electrodes[i];
        if (!electrode->active) {
            electrode->active = true;
            electrode->position = position;
            electrode->radius = ELECTRODE_RADIUS;
            electrode->pulse = (float)GetRandomValue(0, 628) / 100.0f;
            return;
        }
    }
}

void world_spawn_wave(Game *game) {
    clear_wave_entities(game);
    world_reset_player(game);

    int grunt_count = WAVE_START_GRUNTS + (game->wave - 1) * WAVE_GRUNT_STEP;
    if (grunt_count > MAX_GRUNTS) grunt_count = MAX_GRUNTS;

    for (int i = 0; i < grunt_count; i++) {
        spawn_grunt(game, random_edge_position(42.0f));
    }

    if (game->wave >= 2) {
        int spheroid_count = 1 + game->wave / 2;
        if (spheroid_count > 5) spheroid_count = 5;
        for (int i = 0; i < spheroid_count; i++) {
            spawn_spheroid(game, random_edge_position(72.0f));
        }
    }

    if (game->wave >= 7) {
        int quark_count = 1 + (game->wave - 7) / 3;
        if (quark_count > 3) quark_count = 3;
        for (int i = 0; i < quark_count; i++) {
            spawn_quark(game, random_edge_position(84.0f));
        }

        int tank_count = 2 + (game->wave - 7) / 2;
        if (tank_count > 6) tank_count = 6;
        for (int i = 0; i < tank_count; i++) {
            spawn_tank(game, random_edge_position(90.0f));
        }
    }

    int human_count = WAVE_START_HUMANS + game->wave / 2;
    if (human_count > WAVE_HUMAN_MAX) human_count = WAVE_HUMAN_MAX;

    for (int i = 0; i < human_count; i++) {
        spawn_human(game, random_arena_position(80.0f), i % 3);
    }

    int hulk_count = 1 + game->wave / 3;
    if (hulk_count > MAX_HULKS) hulk_count = MAX_HULKS;

    for (int i = 0; i < hulk_count; i++) {
        spawn_hulk(game, random_edge_position(70.0f));
    }

    int electrode_count = WAVE_START_ELECTRODES + game->wave;
    if (electrode_count > WAVE_ELECTRODE_MAX) electrode_count = WAVE_ELECTRODE_MAX;

    for (int i = 0; i < electrode_count; i++) {
        Vector2 position = random_arena_position(70.0f);
        if (Vector2Distance(position, game->player_position) > 135.0f) {
            spawn_electrode(game, position);
        }
    }
}

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

int world_next_human_rescue_score(Game *game) {
    int rescue_index = game->humans_rescued_this_wave + 1;
    if (rescue_index >= 5) {
        return 5000;
    }
    return rescue_index * 1000;
}

static void update_player_and_bullets(Game *game, float dt) {
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

static void update_humans(Game *game, float dt) {
    for (int i = 0; i < MAX_HUMANS; i++) {
        Human *human = &game->humans[i];
        if (!human->active) continue;

        human->retarget_timer -= dt;
        if (human->retarget_timer <= 0.0f) {
            human->velocity = random_human_velocity();
            human->retarget_timer = (float)GetRandomValue(80, 180) / 100.0f;
        }

        human->position.x += human->velocity.x * dt;
        human->position.y += human->velocity.y * dt;

        if (human->position.x < human->radius || human->position.x > WINDOW_WIDTH - human->radius) {
            human->velocity.x *= -1.0f;
            human->position.x = Clamp(human->position.x, human->radius, WINDOW_WIDTH - human->radius);
        }

        if (human->position.y < human->radius || human->position.y > WINDOW_HEIGHT - human->radius) {
            human->velocity.y *= -1.0f;
            human->position.y = Clamp(human->position.y, human->radius, WINDOW_HEIGHT - human->radius);
        }
    }
}

static int find_nearest_human(Game *game, Vector2 position) {
    int best = -1;
    float best_distance = 0.0f;

    for (int i = 0; i < MAX_HUMANS; i++) {
        Human *human = &game->humans[i];
        if (!human->active) continue;

        float distance = Vector2DistanceSqr(position, human->position);
        if (best < 0 || distance < best_distance) {
            best = i;
            best_distance = distance;
        }
    }

    return best;
}

static void update_grunts_and_hulks(Game *game, float dt) {
    for (int i = 0; i < MAX_GRUNTS; i++) {
        Grunt *grunt = &game->grunts[i];
        if (!grunt->active) continue;

        Vector2 to_player = Vector2Subtract(game->player_position, grunt->position);
        if (to_player.x != 0.0f || to_player.y != 0.0f) {
            Vector2 direction = Vector2Normalize(to_player);
            grunt->position.x += direction.x * grunt->speed * dt;
            grunt->position.y += direction.y * grunt->speed * dt;
        }
    }

    for (int i = 0; i < MAX_HULKS; i++) {
        Hulk *hulk = &game->hulks[i];
        if (!hulk->active) continue;

        if (hulk->stun_timer > 0.0f) {
            hulk->stun_timer -= dt;
            continue;
        }

        Vector2 target = game->player_position;
        int human_index = find_nearest_human(game, hulk->position);
        if (human_index >= 0) {
            target = game->humans[human_index].position;
        }

        Vector2 to_target = Vector2Subtract(target, hulk->position);
        if (to_target.x != 0.0f || to_target.y != 0.0f) {
            Vector2 direction = Vector2Normalize(to_target);
            hulk->position.x += direction.x * hulk->speed * dt;
            hulk->position.y += direction.y * hulk->speed * dt;
        }
    }
}

static void update_drifter(Vector2 *position, Vector2 *velocity, float radius, float dt) {
    position->x += velocity->x * dt;
    position->y += velocity->y * dt;

    if (position->x < radius || position->x > WINDOW_WIDTH - radius) {
        velocity->x *= -1.0f;
        position->x = Clamp(position->x, radius, WINDOW_WIDTH - radius);
    }

    if (position->y < radius || position->y > WINDOW_HEIGHT - radius) {
        velocity->y *= -1.0f;
        position->y = Clamp(position->y, radius, WINDOW_HEIGHT - radius);
    }
}

static void update_spawners_and_shooters(Game *game, float dt) {
    for (int i = 0; i < MAX_SPHEROIDS; i++) {
        Spheroid *spheroid = &game->spheroids[i];
        if (!spheroid->active) continue;

        spheroid->retarget_timer -= dt;
        if (spheroid->retarget_timer <= 0.0f) {
            spheroid->velocity = edge_drift_velocity(spheroid->position, spheroid->speed);
            spheroid->retarget_timer = (float)GetRandomValue(120, 240) / 100.0f;
        }

        update_drifter(&spheroid->position, &spheroid->velocity, spheroid->radius, dt);

        spheroid->spawn_timer -= dt;
        if (spheroid->spawn_timer <= 0.0f) {
            spawn_enforcer(game, spheroid->position);
            add_particles(game, spheroid->position, BLUE, 8, 125.0f, 2.5f);
            float base_timer = 3.7f - (float)game->wave * 0.06f;
            if (base_timer < 2.0f) base_timer = 2.0f;
            spheroid->spawn_timer = base_timer + (float)GetRandomValue(0, 100) / 100.0f;
        }
    }

    for (int i = 0; i < MAX_ENFORCERS; i++) {
        Enforcer *enforcer = &game->enforcers[i];
        if (!enforcer->active) continue;

        enforcer->retarget_timer -= dt;
        if (enforcer->retarget_timer <= 0.0f) {
            enforcer->velocity = edge_drift_velocity(enforcer->position, enforcer->speed);
            enforcer->retarget_timer = (float)GetRandomValue(80, 180) / 100.0f;
        }

        update_drifter(&enforcer->position, &enforcer->velocity, enforcer->radius, dt);

        enforcer->shoot_timer -= dt;
        if (enforcer->shoot_timer <= 0.0f) {
            Vector2 target = {
                game->player_position.x + (float)GetRandomValue(-85, 85),
                game->player_position.y + (float)GetRandomValue(-85, 85)
            };
            Vector2 direction = Vector2Subtract(target, enforcer->position);
            if (direction.x == 0.0f && direction.y == 0.0f) {
                direction = random_direction();
            } else {
                direction = Vector2Normalize(direction);
            }
            spawn_projectile(game, PROJECTILE_SPARK, enforcer->position, Vector2Scale(direction, SPARK_SPEED));
            float base_timer = 1.55f - (float)game->wave * 0.025f;
            if (base_timer < 0.75f) base_timer = 0.75f;
            enforcer->shoot_timer = base_timer + (float)GetRandomValue(0, 60) / 100.0f;
        }
    }

    for (int i = 0; i < MAX_QUARKS; i++) {
        Quark *quark = &game->quarks[i];
        if (!quark->active) continue;

        quark->retarget_timer -= dt;
        if (quark->retarget_timer <= 0.0f) {
            quark->velocity = edge_drift_velocity(quark->position, quark->speed);
            quark->retarget_timer = (float)GetRandomValue(140, 280) / 100.0f;
        }

        update_drifter(&quark->position, &quark->velocity, quark->radius, dt);

        quark->spawn_timer -= dt;
        if (quark->spawn_timer <= 0.0f) {
            spawn_tank(game, quark->position);
            add_particles(game, quark->position, RED, 8, 115.0f, 2.5f);
            float base_timer = 4.4f - (float)game->wave * 0.04f;
            if (base_timer < 2.4f) base_timer = 2.4f;
            quark->spawn_timer = base_timer + (float)GetRandomValue(0, 120) / 100.0f;
        }
    }

    for (int i = 0; i < MAX_TANKS; i++) {
        Tank *tank = &game->tanks[i];
        if (!tank->active) continue;

        tank->retarget_timer -= dt;
        if (tank->retarget_timer <= 0.0f) {
            tank->velocity = Vector2Scale(random_direction(), tank->speed);
            tank->retarget_timer = (float)GetRandomValue(130, 260) / 100.0f;
        }

        update_drifter(&tank->position, &tank->velocity, tank->radius, dt);

        tank->shoot_timer -= dt;
        if (tank->shoot_timer <= 0.0f) {
            Vector2 direction = Vector2Subtract(game->player_position, tank->position);
            if (direction.x == 0.0f && direction.y == 0.0f) {
                direction = random_direction();
            } else {
                direction = Vector2Normalize(direction);
            }
            spawn_projectile(game, PROJECTILE_SHELL, tank->position, Vector2Scale(direction, SHELL_SPEED));
            float base_timer = 2.1f - (float)game->wave * 0.025f;
            if (base_timer < 1.05f) base_timer = 1.05f;
            tank->shoot_timer = base_timer + (float)GetRandomValue(0, 90) / 100.0f;
        }
    }
}

static void update_projectiles(Game *game, float dt) {
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

static void update_electrodes_and_float_text(Game *game, float dt) {
    for (int i = 0; i < MAX_ELECTRODES; i++) {
        if (game->electrodes[i].active) {
            game->electrodes[i].pulse += dt * 5.0f;
        }
    }

    for (int i = 0; i < MAX_FLOAT_TEXT; i++) {
        FloatText *text = &game->float_text[i];
        if (!text->active) continue;

        text->position.x += text->velocity.x * dt;
        text->position.y += text->velocity.y * dt;
        text->lifetime -= dt;
        if (text->lifetime <= 0.0f) {
            text->active = false;
        }
    }

    for (int i = 0; i < MAX_PARTICLES; i++) {
        Particle *particle = &game->particles[i];
        if (!particle->active) continue;

        particle->position.x += particle->velocity.x * dt;
        particle->position.y += particle->velocity.y * dt;
        particle->velocity = Vector2Scale(particle->velocity, 0.92f);
        particle->lifetime -= dt;
        if (particle->lifetime <= 0.0f) {
            particle->active = false;
        }
    }

}

static void resolve_bullet_collisions(Game *game) {
    for (int bullet_index = 0; bullet_index < MAX_BULLETS; bullet_index++) {
        Bullet *bullet = &game->bullets[bullet_index];
        if (!bullet->active) continue;

        for (int projectile_index = 0; projectile_index < MAX_PROJECTILES; projectile_index++) {
            EnemyProjectile *projectile = &game->projectiles[projectile_index];
            if (!projectile->active) continue;

            if (circles_overlap(bullet->position, BULLET_RADIUS, projectile->position, projectile->radius)) {
                int score = projectile->type == PROJECTILE_SHELL ? SHELL_SCORE : SPARK_SCORE;
                Color color = projectile->type == PROJECTILE_SHELL ? ORANGE : SKYBLUE;
                add_particles(game, projectile->position, color, 7, 150.0f, 2.5f);
                add_score(game, score);
                bullet->active = false;
                projectile->active = false;
                sound_play(game, SOUND_ENEMY_EXPLODE);
                break;
            }
        }
        if (!bullet->active) continue;

        for (int electrode_index = 0; electrode_index < MAX_ELECTRODES; electrode_index++) {
            Electrode *electrode = &game->electrodes[electrode_index];
            if (!electrode->active) continue;

            if (circles_overlap(bullet->position, BULLET_RADIUS, electrode->position, electrode->radius)) {
                add_particles(game, electrode->position, YELLOW, 8, 160.0f, 3.0f);
                bullet->active = false;
                electrode->active = false;
                sound_play(game, SOUND_ENEMY_EXPLODE);
                break;
            }
        }
        if (!bullet->active) continue;

        for (int hulk_index = 0; hulk_index < MAX_HULKS; hulk_index++) {
            Hulk *hulk = &game->hulks[hulk_index];
            if (!hulk->active) continue;

            if (circles_overlap(bullet->position, BULLET_RADIUS, hulk->position, hulk->radius)) {
                Vector2 shove = Vector2Subtract(hulk->position, bullet->position);
                if (shove.x != 0.0f || shove.y != 0.0f) {
                    shove = Vector2Scale(Vector2Normalize(shove), HULK_KNOCKBACK);
                    hulk->position = Vector2Add(hulk->position, shove);
                }
                hulk->position.x = Clamp(hulk->position.x, hulk->radius, WINDOW_WIDTH - hulk->radius);
                hulk->position.y = Clamp(hulk->position.y, hulk->radius, WINDOW_HEIGHT - hulk->radius);
                hulk->stun_timer = HULK_STUN_TIME;
                bullet->active = false;
                add_particles(game, hulk->position, LIME, 5, 105.0f, 2.5f);
                sound_play(game, SOUND_HULK_HIT);
                break;
            }
        }
        if (!bullet->active) continue;

        for (int spheroid_index = 0; spheroid_index < MAX_SPHEROIDS; spheroid_index++) {
            Spheroid *spheroid = &game->spheroids[spheroid_index];
            if (!spheroid->active) continue;

            if (circles_overlap(bullet->position, BULLET_RADIUS, spheroid->position, spheroid->radius)) {
                add_particles(game, spheroid->position, BLUE, 12, 175.0f, 3.0f);
                bullet->active = false;
                spheroid->active = false;
                add_score(game, SPHEROID_SCORE);
                sound_play(game, SOUND_ENEMY_EXPLODE);
                break;
            }
        }
        if (!bullet->active) continue;

        for (int enforcer_index = 0; enforcer_index < MAX_ENFORCERS; enforcer_index++) {
            Enforcer *enforcer = &game->enforcers[enforcer_index];
            if (!enforcer->active) continue;

            if (circles_overlap(bullet->position, BULLET_RADIUS, enforcer->position, enforcer->radius)) {
                add_particles(game, enforcer->position, SKYBLUE, 10, 165.0f, 3.0f);
                bullet->active = false;
                enforcer->active = false;
                add_score(game, ENFORCER_SCORE);
                sound_play(game, SOUND_ENEMY_EXPLODE);
                break;
            }
        }
        if (!bullet->active) continue;

        for (int quark_index = 0; quark_index < MAX_QUARKS; quark_index++) {
            Quark *quark = &game->quarks[quark_index];
            if (!quark->active) continue;

            if (circles_overlap(bullet->position, BULLET_RADIUS, quark->position, quark->radius)) {
                add_particles(game, quark->position, RAYWHITE, 12, 175.0f, 3.0f);
                bullet->active = false;
                quark->active = false;
                add_score(game, QUARK_SCORE);
                sound_play(game, SOUND_ENEMY_EXPLODE);
                break;
            }
        }
        if (!bullet->active) continue;

        for (int tank_index = 0; tank_index < MAX_TANKS; tank_index++) {
            Tank *tank = &game->tanks[tank_index];
            if (!tank->active) continue;

            if (circles_overlap(bullet->position, BULLET_RADIUS, tank->position, tank->radius)) {
                add_particles(game, tank->position, RED, 10, 165.0f, 3.0f);
                bullet->active = false;
                tank->active = false;
                add_score(game, TANK_SCORE);
                sound_play(game, SOUND_ENEMY_EXPLODE);
                break;
            }
        }
        if (!bullet->active) continue;

        for (int grunt_index = 0; grunt_index < MAX_GRUNTS; grunt_index++) {
            Grunt *grunt = &game->grunts[grunt_index];
            if (!grunt->active) continue;

            if (circles_overlap(bullet->position, BULLET_RADIUS, grunt->position, grunt->radius)) {
                add_particles(game, grunt->position, RED, 10, 180.0f, 3.0f);
                bullet->active = false;
                grunt->active = false;
                add_score(game, GRUNT_SCORE);
                sound_play(game, SOUND_ENEMY_EXPLODE);
                break;
            }
        }
    }
}

static void resolve_human_collisions(Game *game) {
    for (int i = 0; i < MAX_HUMANS; i++) {
        Human *human = &game->humans[i];
        if (!human->active) continue;

        if (circles_overlap(game->player_position, game->player_radius, human->position, human->radius)) {
            int rescue_score = world_next_human_rescue_score(game);
            human->active = false;
            game->humans_rescued_this_wave++;
            add_score(game, rescue_score);
            add_float_text(game, human->position, rescue_score, GOLD);
            add_particles(game, human->position, GOLD, 8, 120.0f, 2.5f);
            sound_play(game, SOUND_HUMAN_RESCUE);
        }
    }

    for (int hulk_index = 0; hulk_index < MAX_HULKS; hulk_index++) {
        Hulk *hulk = &game->hulks[hulk_index];
        if (!hulk->active) continue;

        for (int human_index = 0; human_index < MAX_HUMANS; human_index++) {
            Human *human = &game->humans[human_index];
            if (!human->active) continue;

            if (circles_overlap(hulk->position, hulk->radius, human->position, human->radius)) {
                add_particles(game, human->position, ORANGE, 6, 110.0f, 2.0f);
                human->active = false;
            }
        }
    }
}

static void resolve_grunt_electrode_collisions(Game *game) {
    for (int grunt_index = 0; grunt_index < MAX_GRUNTS; grunt_index++) {
        Grunt *grunt = &game->grunts[grunt_index];
        if (!grunt->active) continue;

        for (int electrode_index = 0; electrode_index < MAX_ELECTRODES; electrode_index++) {
            Electrode *electrode = &game->electrodes[electrode_index];
            if (!electrode->active) continue;

            if (circles_overlap(grunt->position, grunt->radius, electrode->position, electrode->radius)) {
                add_particles(game, grunt->position, RED, 7, 150.0f, 2.5f);
                grunt->active = false;
                break;
            }
        }
    }
}

static void handle_player_death(Game *game) {
    add_particles(game, game->player_position, SKYBLUE, 18, 230.0f, 3.0f);
    game->screen_shake = 0.28f;
    game->screen_flash = 0.18f;
    sound_play(game, SOUND_PLAYER_DIE);
    game->lives--;
    if (game->lives <= 0) {
        game->mode = GAME_MODE_GAME_OVER;
        memset(game->bullets, 0, sizeof(game->bullets));
        memset(game->projectiles, 0, sizeof(game->projectiles));
    } else {
        world_reset_player(game);
    }
}

static void resolve_player_death_collisions(Game *game) {
    if (game->player_invulnerable_timer > 0.0f) {
        return;
    }

    for (int i = 0; i < MAX_PROJECTILES; i++) {
        EnemyProjectile *projectile = &game->projectiles[i];
        if (projectile->active && circles_overlap(game->player_position, game->player_radius, projectile->position, projectile->radius)) {
            handle_player_death(game);
            return;
        }
    }

    for (int i = 0; i < MAX_ELECTRODES; i++) {
        Electrode *electrode = &game->electrodes[i];
        if (electrode->active && circles_overlap(game->player_position, game->player_radius, electrode->position, electrode->radius)) {
            handle_player_death(game);
            return;
        }
    }

    for (int i = 0; i < MAX_HULKS; i++) {
        Hulk *hulk = &game->hulks[i];
        if (hulk->active && circles_overlap(game->player_position, game->player_radius, hulk->position, hulk->radius)) {
            handle_player_death(game);
            return;
        }
    }

    for (int i = 0; i < MAX_SPHEROIDS; i++) {
        Spheroid *spheroid = &game->spheroids[i];
        if (spheroid->active && circles_overlap(game->player_position, game->player_radius, spheroid->position, spheroid->radius)) {
            handle_player_death(game);
            return;
        }
    }

    for (int i = 0; i < MAX_ENFORCERS; i++) {
        Enforcer *enforcer = &game->enforcers[i];
        if (enforcer->active && circles_overlap(game->player_position, game->player_radius, enforcer->position, enforcer->radius)) {
            handle_player_death(game);
            return;
        }
    }

    for (int i = 0; i < MAX_QUARKS; i++) {
        Quark *quark = &game->quarks[i];
        if (quark->active && circles_overlap(game->player_position, game->player_radius, quark->position, quark->radius)) {
            handle_player_death(game);
            return;
        }
    }

    for (int i = 0; i < MAX_TANKS; i++) {
        Tank *tank = &game->tanks[i];
        if (tank->active && circles_overlap(game->player_position, game->player_radius, tank->position, tank->radius)) {
            handle_player_death(game);
            return;
        }
    }

    for (int i = 0; i < MAX_GRUNTS; i++) {
        Grunt *grunt = &game->grunts[i];
        if (grunt->active && circles_overlap(game->player_position, game->player_radius, grunt->position, grunt->radius)) {
            handle_player_death(game);
            return;
        }
    }
}

void world_update_playing(Game *game, float dt) {
    update_player_and_bullets(game, dt);
    update_humans(game, dt);
    update_grunts_and_hulks(game, dt);
    update_spawners_and_shooters(game, dt);
    update_projectiles(game, dt);
    update_electrodes_and_float_text(game, dt);

    resolve_bullet_collisions(game);
    resolve_human_collisions(game);
    resolve_grunt_electrode_collisions(game);

    if (world_count_active_grunts(game) == 0 &&
        world_count_active_spheroids(game) == 0 &&
        world_count_active_enforcers(game) == 0 &&
        world_count_active_quarks(game) == 0 &&
        world_count_active_tanks(game) == 0) {
        sound_play(game, SOUND_WAVE_CLEAR);
        game->screen_flash = 0.12f;
        game->wave++;
        world_spawn_wave(game);
        return;
    }

    resolve_player_death_collisions(game);
}

static void draw_arena_grid(void) {
    Color major = (Color){ 42, 54, 68, 120 };
    Color minor = (Color){ 30, 38, 50, 90 };

    for (int x = 0; x <= WINDOW_WIDTH; x += 40) {
        DrawLine(x, 0, x, WINDOW_HEIGHT, (x % 120 == 0) ? major : minor);
    }

    for (int y = 0; y <= WINDOW_HEIGHT; y += 40) {
        DrawLine(0, y, WINDOW_WIDTH, y, (y % 120 == 0) ? major : minor);
    }
}

static void draw_texture_centered(Game *game, TextureID id, Vector2 position, float size, Color tint) {
    if (!texture_is_ready(game, id)) {
        return;
    }

    Texture2D texture = game->textures[id];
    Rectangle source = { 0.0f, 0.0f, (float)texture.width, (float)texture.height };
    Rectangle dest = { position.x, position.y, size, size };
    Vector2 origin = { size * 0.5f, size * 0.5f };
    DrawTexturePro(texture, source, dest, origin, 0.0f, tint);
}

void world_draw_playfield(Game *game) {
    draw_arena_grid();

    for (int i = 0; i < MAX_ELECTRODES; i++) {
        Electrode electrode = game->electrodes[i];
        if (!electrode.active) continue;

        float pulse = 0.5f + 0.5f * sinf(electrode.pulse);
        Color glow = (Color){ 255, 190, 42, (unsigned char)(70 + pulse * 70) };
        DrawCircleV(electrode.position, electrode.radius + 7.0f + pulse * 3.0f, glow);
        if (texture_is_ready(game, TEXTURE_ELECTRODE)) {
            draw_texture_centered(game, TEXTURE_ELECTRODE, electrode.position, 34.0f + pulse * 3.0f, WHITE);
        } else {
            DrawCircleLines((int)electrode.position.x, (int)electrode.position.y, electrode.radius + 5.0f, ORANGE);
            DrawLineEx((Vector2){ electrode.position.x - 11.0f, electrode.position.y },
                (Vector2){ electrode.position.x + 11.0f, electrode.position.y }, 4.0f, YELLOW);
            DrawLineEx((Vector2){ electrode.position.x, electrode.position.y - 11.0f },
                (Vector2){ electrode.position.x, electrode.position.y + 11.0f }, 4.0f, YELLOW);
        }
    }

    for (int i = 0; i < MAX_HUMANS; i++) {
        Human human = game->humans[i];
        if (!human.active) continue;

        Color color = GOLD;
        TextureID texture_id = TEXTURE_HUMAN_MOMMY;
        if (human.type == 1) {
            color = ORANGE;
            texture_id = TEXTURE_HUMAN_DADDY;
        } else if (human.type == 2) {
            color = LIME;
            texture_id = TEXTURE_HUMAN_MIKEY;
        }

        DrawCircleV(human.position, human.radius + 5.0f, Fade(color, 0.35f));
        if (texture_is_ready(game, texture_id)) {
            draw_texture_centered(game, texture_id, human.position, 28.0f, WHITE);
        } else {
            DrawCircleV(human.position, human.radius, color);
            DrawCircleV((Vector2){ human.position.x, human.position.y - 3.0f }, 3.0f, RAYWHITE);
        }
    }

    for (int i = 0; i < MAX_HULKS; i++) {
        Hulk hulk = game->hulks[i];
        if (!hulk.active) continue;

        DrawCircleV(hulk.position, hulk.radius + 6.0f, (Color){ 60, 255, 120, 70 });
        if (texture_is_ready(game, TEXTURE_HULK)) {
            draw_texture_centered(game, TEXTURE_HULK, hulk.position, 52.0f, WHITE);
        } else {
            DrawCircleV(hulk.position, hulk.radius, (Color){ 38, 176, 82, 255 });
            DrawRectangle((int)(hulk.position.x - 12.0f), (int)(hulk.position.y - 8.0f), 24, 10, DARKGREEN);
        }
        if (hulk.stun_timer > 0.0f) {
            DrawCircleLines((int)hulk.position.x, (int)hulk.position.y, hulk.radius + 10.0f, RAYWHITE);
        }
    }

    for (int i = 0; i < MAX_SPHEROIDS; i++) {
        Spheroid spheroid = game->spheroids[i];
        if (!spheroid.active) continue;

        float pulse = 0.5f + 0.5f * sinf(GetTime() * 5.0f + (float)i);
        DrawCircleV(spheroid.position, spheroid.radius + 8.0f + pulse * 2.0f, (Color){ 70, 115, 255, 65 });
        if (texture_is_ready(game, TEXTURE_SPHEROID)) {
            draw_texture_centered(game, TEXTURE_SPHEROID, spheroid.position, 40.0f, WHITE);
        } else {
            DrawCircleLines((int)spheroid.position.x, (int)spheroid.position.y, spheroid.radius + 3.0f, BLUE);
            DrawCircleV(spheroid.position, spheroid.radius, (Color){ 64, 95, 240, 255 });
            DrawCircleV(spheroid.position, spheroid.radius * 0.45f, BLACK);
        }
    }

    for (int i = 0; i < MAX_ENFORCERS; i++) {
        Enforcer enforcer = game->enforcers[i];
        if (!enforcer.active) continue;

        DrawCircleV(enforcer.position, enforcer.radius + 7.0f, (Color){ 68, 200, 255, 60 });
        if (texture_is_ready(game, TEXTURE_ENFORCER)) {
            draw_texture_centered(game, TEXTURE_ENFORCER, enforcer.position, 36.0f, WHITE);
        } else {
            DrawPoly(enforcer.position, 3, enforcer.radius + 4.0f, 90.0f, SKYBLUE);
            DrawPoly(enforcer.position, 3, enforcer.radius - 2.0f, 90.0f, DARKBLUE);
            DrawCircleV(enforcer.position, 4.0f, RAYWHITE);
        }
    }

    for (int i = 0; i < MAX_QUARKS; i++) {
        Quark quark = game->quarks[i];
        if (!quark.active) continue;

        float pulse = 0.5f + 0.5f * sinf(GetTime() * 4.0f + (float)i);
        DrawCircleV(quark.position, quark.radius + 8.0f + pulse * 2.0f, (Color){ 245, 245, 245, 60 });
        if (texture_is_ready(game, TEXTURE_QUARK)) {
            draw_texture_centered(game, TEXTURE_QUARK, quark.position, 40.0f, WHITE);
        } else {
            DrawRectanglePro((Rectangle){ quark.position.x, quark.position.y, 30.0f, 30.0f },
                (Vector2){ 15.0f, 15.0f }, 45.0f, RAYWHITE);
            DrawRectanglePro((Rectangle){ quark.position.x, quark.position.y, 16.0f, 16.0f },
                (Vector2){ 8.0f, 8.0f }, 45.0f, BLACK);
        }
    }

    for (int i = 0; i < MAX_TANKS; i++) {
        Tank tank = game->tanks[i];
        if (!tank.active) continue;

        DrawCircleV(tank.position, tank.radius + 7.0f, (Color){ 255, 64, 42, 60 });
        if (texture_is_ready(game, TEXTURE_TANK)) {
            draw_texture_centered(game, TEXTURE_TANK, tank.position, 42.0f, WHITE);
        } else {
            DrawRectangle((int)(tank.position.x - 15.0f), (int)(tank.position.y - 12.0f), 30, 24, MAROON);
            DrawRectangle((int)(tank.position.x - 9.0f), (int)(tank.position.y - 7.0f), 18, 14, RED);
            Vector2 barrel = tank.velocity;
            if (barrel.x == 0.0f && barrel.y == 0.0f) {
                barrel = (Vector2){ 0.0f, -1.0f };
            } else {
                barrel = Vector2Normalize(barrel);
            }
            DrawLineEx(tank.position, Vector2Add(tank.position, Vector2Scale(barrel, 22.0f)), 4.0f, ORANGE);
        }
    }

    for (int i = 0; i < MAX_GRUNTS; i++) {
        Grunt grunt = game->grunts[i];
        if (!grunt.active) continue;

        DrawCircleV(grunt.position, grunt.radius + 5.0f, (Color){ 255, 62, 103, 65 });
        if (texture_is_ready(game, TEXTURE_GRUNT)) {
            draw_texture_centered(game, TEXTURE_GRUNT, grunt.position, 34.0f, WHITE);
        } else {
            DrawCircleV(grunt.position, grunt.radius, (Color){ 230, 40, 72, 255 });
            DrawCircleV((Vector2){ grunt.position.x - 4.0f, grunt.position.y - 3.0f }, 3.0f, BLACK);
            DrawCircleV((Vector2){ grunt.position.x + 4.0f, grunt.position.y - 3.0f }, 3.0f, BLACK);
        }
    }

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

    for (int i = 0; i < MAX_BULLETS; i++) {
        Bullet bullet = game->bullets[i];
        if (!bullet.active) continue;

        DrawCircleV(bullet.position, BULLET_RADIUS + 3.0f, (Color){ 77, 214, 255, 70 });
        DrawCircleV(bullet.position, BULLET_RADIUS, RAYWHITE);
    }

    bool blink_off = game->player_invulnerable_timer > 0.0f && ((int)(game->player_invulnerable_timer * 12.0f) % 2) == 0;
    if (!blink_off || game->mode != GAME_MODE_PLAYING) {
        DrawCircleV(game->player_position, game->player_radius + 6.0f, (Color){ 77, 214, 255, 80 });
        if (texture_is_ready(game, TEXTURE_PLAYER)) {
            draw_texture_centered(game, TEXTURE_PLAYER, game->player_position, 36.0f, WHITE);
        } else {
            DrawCircleV(game->player_position, game->player_radius, game->player_color);
            DrawCircleV(game->player_position, 4.0f, RAYWHITE);
        }
    }

    for (int i = 0; i < MAX_PARTICLES; i++) {
        Particle particle = game->particles[i];
        if (!particle.active) continue;

        float alpha = Clamp(particle.lifetime / particle.max_lifetime, 0.0f, 1.0f);
        DrawCircleV(particle.position, particle.radius, Fade(particle.color, alpha));
    }

    for (int i = 0; i < MAX_FLOAT_TEXT; i++) {
        FloatText text = game->float_text[i];
        if (!text.active) continue;

        float alpha = Clamp(text.lifetime / FLOAT_TEXT_LIFETIME, 0.0f, 1.0f);
        const char *label = text.value > 0 ? TextFormat("+%d", text.value) : "EXTRA LIFE";
        DrawText(label, (int)text.position.x - MeasureText(label, 18) / 2, (int)text.position.y, 18, Fade(text.color, alpha));
    }
}

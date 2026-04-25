#include "world_internal.h"
#include "raymath.h"

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

static Vector2 random_human_velocity(void) {
    return Vector2Scale(random_direction(), HUMAN_SPEED);
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

void world_spawn_grunt(Game *game, Vector2 position) {
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

void world_spawn_hulk(Game *game, Vector2 position) {
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

void world_spawn_spheroid(Game *game, Vector2 position) {
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

void world_spawn_quark(Game *game, Vector2 position) {
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

void world_spawn_tank(Game *game, Vector2 position) {
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

void world_spawn_human(Game *game, Vector2 position, int type) {
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

void world_spawn_electrode(Game *game, Vector2 position) {
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

void world_update_humans(Game *game, float dt) {
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

void world_update_grunts_and_hulks(Game *game, float dt) {
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

void world_update_spawners_and_shooters(Game *game, float dt) {
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
            world_add_particles(game, spheroid->position, BLUE, 8, 125.0f, 2.5f);
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
            world_spawn_projectile(game, PROJECTILE_SPARK, enforcer->position, Vector2Scale(direction, SPARK_SPEED));
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
            world_spawn_tank(game, quark->position);
            world_add_particles(game, quark->position, RED, 8, 115.0f, 2.5f);
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
            world_spawn_projectile(game, PROJECTILE_SHELL, tank->position, Vector2Scale(direction, SHELL_SPEED));
            float base_timer = 2.1f - (float)game->wave * 0.025f;
            if (base_timer < 1.05f) base_timer = 1.05f;
            tank->shoot_timer = base_timer + (float)GetRandomValue(0, 90) / 100.0f;
        }
    }
}

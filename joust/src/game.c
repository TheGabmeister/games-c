#include "game.h"
#include "sounds.h"
#include "raymath.h"
#include <math.h>
#include <string.h>

#define EGG_RADIUS             12.0f
#define EGG_GRAVITY            780.0f
#define EGG_HATCH_WAIT         5.0f
#define EGG_HATCH_WARNING      2.0f
#define WAVE_CLEAR_TIME        1.8f
#define PLAYER_DEAD_TIME       1.4f
#define ENEMY_SPAWN_GRACE      0.75f
#define PARTICLE_TIME          0.45f

typedef struct EnemyTuning {
    float accel;
    float max_speed_x;
    float flap_velocity;
    float flap_cooldown;
    float think_interval;
    int   score;
    Color color;
    TextureID texture;
} EnemyTuning;

static const EnemyTuning ENEMY_TUNING[ENEMY_COUNT] = {
    [ENEMY_GRUNT] = {
        .accel = 560.0f, .max_speed_x = 190.0f,
        .flap_velocity = -365.0f, .flap_cooldown = 0.48f,
        .think_interval = 0.70f, .score = 500,
        .color = { 180, 113, 226, 255 },
        .texture = TEXTURE_ENEMY_GRUNT,
    },
    [ENEMY_HUNTER] = {
        .accel = 690.0f, .max_speed_x = 245.0f,
        .flap_velocity = -405.0f, .flap_cooldown = 0.34f,
        .think_interval = 0.45f, .score = 750,
        .color = { 238, 90, 75, 255 },
        .texture = TEXTURE_ENEMY_HUNTER,
    },
    [ENEMY_CHAMPION] = {
        .accel = 760.0f, .max_speed_x = 285.0f,
        .flap_velocity = -440.0f, .flap_cooldown = 0.25f,
        .think_interval = 0.32f, .score = 1000,
        .color = { 244, 202, 82, 255 },
        .texture = TEXTURE_ENEMY_CHAMPION,
    },
};

static const Platform INITIAL_PLATFORMS[] = {
    { {  90.0f, 230.0f, 260.0f, 28.0f } },
    { { 850.0f, 230.0f, 260.0f, 28.0f } },
    { { 430.0f, 420.0f, 340.0f, 28.0f } },
    { { 120.0f, 640.0f, 300.0f, 28.0f } },
    { { 780.0f, 640.0f, 300.0f, 28.0f } },
};

static const Vector2 ENEMY_SPAWN_POINTS[] = {
    { 160.0f, 150.0f },
    { 1040.0f, 150.0f },
    { 260.0f, 520.0f },
    { 940.0f, 520.0f },
    { 600.0f, 320.0f },
};

static float frame_dt(void) {
    // Cap dt so collisions stay sane during hitches/debug breaks.
    float dt = GetFrameTime();
    if (dt > 1.0f / 30.0f) {
        dt = 1.0f / 30.0f;
    }
    return dt;
}

static bool start_pressed(void) {
    return IsKeyPressed(KEY_ENTER) ||
        (IsGamepadAvailable(0) && IsGamepadButtonPressed(0, GAMEPAD_BUTTON_MIDDLE_RIGHT));
}

static bool pause_pressed(void) {
    return IsKeyPressed(KEY_P) ||
        (IsGamepadAvailable(0) && IsGamepadButtonPressed(0, GAMEPAD_BUTTON_MIDDLE_RIGHT));
}

static bool flap_pressed(void) {
    return IsKeyPressed(KEY_SPACE) || IsKeyPressed(KEY_W) || IsKeyPressed(KEY_UP) ||
        (IsGamepadAvailable(0) && IsGamepadButtonPressed(0, GAMEPAD_BUTTON_RIGHT_FACE_DOWN));
}

static float input_axis_x(void) {
    float axis = 0.0f;
    if (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT)) axis -= 1.0f;
    if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT)) axis += 1.0f;
    if (IsGamepadAvailable(0)) {
        float stick = GetGamepadAxisMovement(0, GAMEPAD_AXIS_LEFT_X);
        if (fabsf(stick) > 0.25f) {
            axis = stick;
        }
    }
    return Clamp(axis, -1.0f, 1.0f);
}

static float wrapped_delta_x(float from, float to) {
    float dx = to - from;
    if (dx > WINDOW_WIDTH * 0.5f) dx -= WINDOW_WIDTH;
    if (dx < -WINDOW_WIDTH * 0.5f) dx += WINDOW_WIDTH;
    return dx;
}

static void wrap_actor(Actor *actor) {
    if (actor->position.x < -actor->radius) {
        actor->position.x = WINDOW_WIDTH + actor->radius;
    } else if (actor->position.x > WINDOW_WIDTH + actor->radius) {
        actor->position.x = -actor->radius;
    }
}

static void wrap_egg(Egg *egg) {
    if (egg->position.x < -EGG_RADIUS) {
        egg->position.x = WINDOW_WIDTH + EGG_RADIUS;
    } else if (egg->position.x > WINDOW_WIDTH + EGG_RADIUS) {
        egg->position.x = -EGG_RADIUS;
    }
}

static void bounce_actors(Actor *a, Actor *b) {
    float dx = wrapped_delta_x(a->position.x, b->position.x);
    float dir = dx >= 0.0f ? 1.0f : -1.0f;
    a->velocity.x = -dir * JOUST_BOUNCE_SPEED;
    b->velocity.x = dir * JOUST_BOUNCE_SPEED;
    a->velocity.y = -JOUST_BOUNCE_SPEED * 0.55f;
    b->velocity.y = -JOUST_BOUNCE_SPEED * 0.55f;
}

static void reset_actor(Actor *actor, Vector2 position, float radius) {
    memset(actor, 0, sizeof(*actor));
    actor->previous_position = position;
    actor->position = position;
    actor->radius = radius;
    actor->facing = 1;
    actor->alive = true;
}

static void setup_arena(Game *game) {
    game->platform_count = (int)(sizeof(INITIAL_PLATFORMS) / sizeof(INITIAL_PLATFORMS[0]));
    for (int i = 0; i < game->platform_count; i++) {
        game->platforms[i] = INITIAL_PLATFORMS[i];
    }
    game->lava = (Rectangle){ 0.0f, WINDOW_HEIGHT - LAVA_HEIGHT, WINDOW_WIDTH, LAVA_HEIGHT };
}

static void spawn_particles(Game *game, Vector2 position, Color color, int count) {
    for (int i = 0; i < MAX_PARTICLES && count > 0; i++) {
        Particle *particle = &game->particles[i];
        if (!particle->active) {
            float angle = (float)GetRandomValue(0, 359) * DEG2RAD;
            float speed = (float)GetRandomValue(PARTICLE_SPEED_MIN, PARTICLE_SPEED_MAX);
            particle->active = true;
            particle->position = position;
            particle->velocity = (Vector2){ cosf(angle) * speed, sinf(angle) * speed };
            particle->radius = (float)GetRandomValue(PARTICLE_RADIUS_MIN, PARTICLE_RADIUS_MAX);
            particle->lifetime = PARTICLE_TIME;
            particle->max_lifetime = PARTICLE_TIME;
            particle->color = color;
            count--;
        }
    }
}

static void clear_wave_entities(Game *game) {
    memset(game->enemies, 0, sizeof(game->enemies));
    memset(game->eggs, 0, sizeof(game->eggs));
}

static void spawn_enemy(Game *game, EnemyType type, Vector2 position) {
    for (int i = 0; i < MAX_ENEMIES; i++) {
        Enemy *enemy = &game->enemies[i];
        if (!enemy->actor.alive) {
            memset(enemy, 0, sizeof(*enemy));
            reset_actor(&enemy->actor, position, ENEMY_RADIUS);
            enemy->actor.facing = position.x < WINDOW_WIDTH * 0.5f ? 1 : -1;
            enemy->type = type;
            enemy->think_timer = 0.1f;
            enemy->target_y = position.y;
            enemy->spawn_grace_timer = ENEMY_SPAWN_GRACE;
            enemy->escape_dir = enemy->actor.facing;
            return;
        }
    }
}

static void spawn_egg(Game *game, Vector2 position, EnemyType hatch_type) {
    for (int i = 0; i < MAX_EGGS; i++) {
        Egg *egg = &game->eggs[i];
        if (!egg->active) {
            memset(egg, 0, sizeof(*egg));
            egg->active = true;
            egg->previous_position = position;
            egg->position = position;
            egg->velocity = (Vector2){ (float)GetRandomValue(-70, 70), -120.0f };
            egg->state = EGG_FALLING;
            egg->timer = EGG_HATCH_WAIT;
            egg->hatch_type = hatch_type;
            return;
        }
    }
}

static int active_enemy_count(const Game *game) {
    int count = 0;
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (game->enemies[i].actor.alive) count++;
    }
    return count;
}

static int active_egg_count(const Game *game) {
    int count = 0;
    for (int i = 0; i < MAX_EGGS; i++) {
        if (game->eggs[i].active) count++;
    }
    return count;
}

static void respawn_player(Game *game) {
    reset_actor(&game->player, (Vector2){ WINDOW_WIDTH * 0.5f, 150.0f }, PLAYER_RADIUS);
    game->player.invuln_timer = PLAYER_INVULN_TIME;
    game->combo = 0;
}

static void setup_wave(Game *game) {
    clear_wave_entities(game);
    respawn_player(game);

    int grunts = 3 + game->wave / 2;
    int hunters = game->wave >= 3 ? 1 + (game->wave - 3) / 2 : 0;
    int champions = game->wave >= 5 ? (game->wave - 4) / 3 : 0;
    if (champions > 3) champions = 3;
    if (hunters > 6) hunters = 6;
    if (grunts + hunters + champions > MAX_ENEMIES) {
        grunts = MAX_ENEMIES - hunters - champions;
        if (grunts < 0) grunts = 0;
    }

    int spawn_index = 0;
    int spawn_count = (int)(sizeof(ENEMY_SPAWN_POINTS) / sizeof(ENEMY_SPAWN_POINTS[0]));
    for (int i = 0; i < grunts; i++) {
        spawn_enemy(game, ENEMY_GRUNT, ENEMY_SPAWN_POINTS[spawn_index++ % spawn_count]);
    }
    for (int i = 0; i < hunters; i++) {
        spawn_enemy(game, ENEMY_HUNTER, ENEMY_SPAWN_POINTS[spawn_index++ % spawn_count]);
    }
    for (int i = 0; i < champions; i++) {
        spawn_enemy(game, ENEMY_CHAMPION, ENEMY_SPAWN_POINTS[spawn_index++ % spawn_count]);
    }
}

static void start_new_game(Game *game, Resources *res) {
    game->score = 0;
    game->lives = PLAYER_LIVES;
    game->wave = 1;
    game->combo = 0;
    game->mode = GAME_MODE_PLAYING;
    game->mode_timer = 0.0f;
    memset(game->particles, 0, sizeof(game->particles));
    setup_wave(game);
    sound_play(res, SOUND_START);
}

static void kill_player(Game *game, Resources *res) {
    if (game->mode != GAME_MODE_PLAYING || game->player.invuln_timer > 0.0f) {
        return;
    }

    game->lives--;
    game->combo = 0;
    game->player.alive = false;
    game->mode = GAME_MODE_PLAYER_DEAD;
    game->mode_timer = PLAYER_DEAD_TIME;
    spawn_particles(game, game->player.position, (Color){ 92, 205, 255, 255 }, 28);
    sound_play(res, SOUND_PLAYER_DIE);
    if (game->score > game->high_score) {
        game->high_score = game->score;
    }
}

static void defeat_enemy(Game *game, Resources *res, Enemy *enemy) {
    enemy->actor.alive = false;
    game->combo++;
    int multiplier = game->combo > 1 ? game->combo : 1;
    if (multiplier > 5) multiplier = 5;
    game->score += ENEMY_TUNING[enemy->type].score * multiplier;
    if (game->score > game->high_score) {
        game->high_score = game->score;
    }
    spawn_egg(game, enemy->actor.position, enemy->type);
    spawn_particles(game, enemy->actor.position, ENEMY_TUNING[enemy->type].color, 22);
    sound_play(res, SOUND_JOUST_WIN);
}

// Swept platform landing: treat the actor's previous->current path as a vertical
// segment and check intersection with the platform top, so high-velocity falls
// don't tunnel through thin platforms.
static void resolve_actor_platforms(Game *game, Actor *actor) {
    actor->grounded = false;
    if (actor->velocity.y < 0.0f) return;

    float prev_bottom = actor->previous_position.y + actor->radius;
    float curr_bottom = actor->position.y + actor->radius;

    for (int i = 0; i < game->platform_count; i++) {
        Rectangle bounds = game->platforms[i].bounds;

        // Was the actor's bottom above the platform top last frame (within tolerance)?
        if (prev_bottom > bounds.y + LANDING_TOLERANCE) continue;
        // Did it cross the platform top this frame?
        if (curr_bottom < bounds.y) continue;
        // Is the actor horizontally over the platform now?
        if (actor->position.x + actor->radius < bounds.x) continue;
        if (actor->position.x - actor->radius > bounds.x + bounds.width) continue;

        actor->position.y = bounds.y - actor->radius;
        actor->velocity.y = 0.0f;
        actor->grounded = true;
        return;
    }
}

// Returns true if the egg landed on a platform this frame.
static bool resolve_egg_platforms(Game *game, Egg *egg) {
    if (egg->velocity.y < 0.0f) return false;

    float prev_bottom = egg->previous_position.y + EGG_RADIUS;
    float curr_bottom = egg->position.y + EGG_RADIUS;

    for (int i = 0; i < game->platform_count; i++) {
        Rectangle bounds = game->platforms[i].bounds;

        if (prev_bottom > bounds.y + LANDING_TOLERANCE) continue;
        if (curr_bottom < bounds.y) continue;
        if (egg->position.x + EGG_RADIUS < bounds.x) continue;
        if (egg->position.x - EGG_RADIUS > bounds.x + bounds.width) continue;

        egg->position.y = bounds.y - EGG_RADIUS;
        egg->velocity = (Vector2){ 0.0f, 0.0f };
        return true;
    }
    return false;
}

static void update_player(Game *game, Resources *res, float dt) {
    Actor *player = &game->player;
    player->previous_position = player->position;
    if (player->invuln_timer > 0.0f) player->invuln_timer -= dt;
    if (player->flap_cooldown > 0.0f) player->flap_cooldown -= dt;

    float axis = input_axis_x();
    if (axis != 0.0f) {
        player->velocity.x += axis * PLAYER_ACCEL * dt;
        player->facing = axis < 0.0f ? -1 : 1;
    } else {
        player->velocity.x *= powf(PLAYER_AIR_DRAG, dt * TARGET_FPS);
    }

    if (flap_pressed() && player->flap_cooldown <= 0.0f) {
        player->velocity.y = PLAYER_FLAP_VELOCITY;
        player->flap_cooldown = PLAYER_FLAP_COOLDOWN;
        player->grounded = false;
        sound_play(res, SOUND_FLAP);
    }

    player->velocity.y += PLAYER_GRAVITY * dt;
    player->velocity.x = Clamp(player->velocity.x, -PLAYER_MAX_SPEED_X, PLAYER_MAX_SPEED_X);
    player->velocity.y = Clamp(player->velocity.y, -PLAYER_MAX_SPEED_Y, PLAYER_MAX_SPEED_Y);
    player->position.x += player->velocity.x * dt;
    player->position.y += player->velocity.y * dt;

    if (player->position.y < player->radius) {
        player->position.y = player->radius;
        player->velocity.y = 0.0f;
    }
    wrap_actor(player);
    resolve_actor_platforms(game, player);
    if (player->grounded) {
        game->combo = 0;
    }
}

static void update_enemy(Game *game, Enemy *enemy, float dt) {
    Actor *actor = &enemy->actor;
    const EnemyTuning *tuning = &ENEMY_TUNING[enemy->type];
    actor->previous_position = actor->position;
    if (actor->flap_cooldown > 0.0f) actor->flap_cooldown -= dt;
    if (enemy->spawn_grace_timer > 0.0f) enemy->spawn_grace_timer -= dt;
    if (enemy->lava_recover_timer > 0.0f) enemy->lava_recover_timer -= dt;

    enemy->think_timer -= dt;
    if (enemy->think_timer <= 0.0f) {
        enemy->think_timer = tuning->think_interval;
        float offset = (float)GetRandomValue(-80, 80);
        if (enemy->type == ENEMY_HUNTER) offset = (float)GetRandomValue(-110, 30);
        if (enemy->type == ENEMY_CHAMPION) offset = (float)GetRandomValue(-140, -30);
        enemy->target_y = Clamp(game->player.position.y + offset,
            ENEMY_AI_TARGET_Y_MIN,
            WINDOW_HEIGHT - LAVA_HEIGHT - ENEMY_AI_TARGET_Y_PAD);
    }

    float dx = wrapped_delta_x(actor->position.x, game->player.position.x);
    int desired_dir = dx < -ENEMY_AI_DEADZONE_X ? -1 : 1;
    if (fabsf(dx) <= ENEMY_AI_DEADZONE_X) desired_dir = actor->facing;

    bool trying_to_climb = actor->position.y > enemy->target_y + ENEMY_AI_CLIMB_MARGIN;
    if (trying_to_climb && fabsf(actor->position.y - actor->previous_position.y) < ENEMY_AI_STUCK_EPS) {
        enemy->stuck_timer += dt;
    } else {
        enemy->stuck_timer -= dt * ENEMY_AI_STUCK_DECAY;
        if (enemy->stuck_timer < 0.0f) enemy->stuck_timer = 0.0f;
    }
    if (enemy->stuck_timer > ENEMY_AI_STUCK_ENTER) {
        desired_dir = enemy->escape_dir;
        if (enemy->stuck_timer > ENEMY_AI_STUCK_FLIP) {
            enemy->stuck_timer = 0.0f;
            enemy->escape_dir = -enemy->escape_dir;
        }
    }

    actor->velocity.x += (float)desired_dir * tuning->accel * dt;
    actor->facing = desired_dir;

    if (actor->position.y > enemy->target_y && actor->flap_cooldown <= 0.0f) {
        actor->velocity.y = tuning->flap_velocity;
        actor->flap_cooldown = tuning->flap_cooldown;
    }

    actor->velocity.y += PLAYER_GRAVITY * dt;
    actor->velocity.x *= powf(ENEMY_AIR_DRAG, dt * TARGET_FPS);
    actor->velocity.x = Clamp(actor->velocity.x, -tuning->max_speed_x, tuning->max_speed_x);
    actor->velocity.y = Clamp(actor->velocity.y, -PLAYER_MAX_SPEED_Y, PLAYER_MAX_SPEED_Y);
    actor->position.x += actor->velocity.x * dt;
    actor->position.y += actor->velocity.y * dt;

    if (actor->position.y < actor->radius) {
        actor->position.y = actor->radius;
        actor->velocity.y = 0.0f;
    }
    wrap_actor(actor);
    resolve_actor_platforms(game, actor);
}

static void update_eggs(Game *game, Resources *res, float dt) {
    for (int i = 0; i < MAX_EGGS; i++) {
        Egg *egg = &game->eggs[i];
        if (!egg->active) continue;

        egg->previous_position = egg->position;
        if (egg->state == EGG_FALLING) {
            egg->velocity.y += EGG_GRAVITY * dt;
            egg->position.x += egg->velocity.x * dt;
            egg->position.y += egg->velocity.y * dt;
            wrap_egg(egg);
            if (resolve_egg_platforms(game, egg)) {
                egg->state = EGG_RESTING;
                egg->timer = EGG_HATCH_WAIT;
                egg->on_lava = false;
            } else if (CheckCollisionCircleRec(egg->position, EGG_RADIUS, game->lava)) {
                egg->position.y = game->lava.y - EGG_RADIUS;
                egg->velocity = (Vector2){ 0.0f, 0.0f };
                egg->state = EGG_RESTING;
                egg->timer = EGG_HATCH_WAIT * 0.5f;
                egg->on_lava = true;
            }
        } else {
            egg->timer -= dt;
            if (egg->state == EGG_RESTING && egg->timer <= 0.0f) {
                egg->state = EGG_HATCHING;
                egg->timer = EGG_HATCH_WARNING;
                sound_play(res, SOUND_EGG_HATCH);
            } else if (egg->state == EGG_HATCHING && egg->timer <= 0.0f) {
                // Spawn the new enemy clear of lava so it doesn't immediately
                // re-collide and loop on the lava-bounce response.
                Vector2 spawn_pos = egg->position;
                if (egg->on_lava) {
                    spawn_pos.y = game->lava.y - ENEMY_RADIUS - 2.0f;
                }
                int before = active_enemy_count(game);
                spawn_enemy(game, egg->hatch_type, spawn_pos);
                if (active_enemy_count(game) > before) {
                    egg->active = false;
                    spawn_particles(game, egg->position, (Color){ 255, 235, 135, 255 }, 14);
                } else {
                    egg->timer = 0.2f;
                }
            }
        }
    }
}

static void update_particles(Game *game, float dt) {
    for (int i = 0; i < MAX_PARTICLES; i++) {
        Particle *particle = &game->particles[i];
        if (!particle->active) continue;
        particle->lifetime -= dt;
        if (particle->lifetime <= 0.0f) {
            particle->active = false;
            continue;
        }
        particle->velocity.y += PARTICLE_GRAVITY * dt;
        particle->position.x += particle->velocity.x * dt;
        particle->position.y += particle->velocity.y * dt;
    }
}

static void resolve_jousts(Game *game, Resources *res) {
    for (int i = 0; i < MAX_ENEMIES; i++) {
        Enemy *enemy = &game->enemies[i];
        if (!enemy->actor.alive || enemy->spawn_grace_timer > 0.0f) continue;
        if (!CheckCollisionCircles(game->player.position, game->player.radius, enemy->actor.position, enemy->actor.radius)) {
            continue;
        }

        if (game->player.invuln_timer > 0.0f) {
            bounce_actors(&game->player, &enemy->actor);
            sound_play(res, SOUND_JOUST_BOUNCE);
            continue;
        }

        float vertical_delta = enemy->actor.position.y - game->player.position.y;
        if (vertical_delta > JOUST_WIN_HEIGHT) {
            defeat_enemy(game, res, enemy);
            game->player.velocity.y = PLAYER_FLAP_VELOCITY * 0.45f;
        } else if (-vertical_delta > JOUST_WIN_HEIGHT) {
            kill_player(game, res);
            return;
        } else {
            bounce_actors(&game->player, &enemy->actor);
            sound_play(res, SOUND_JOUST_BOUNCE);
        }
    }

    for (int i = 0; i < MAX_ENEMIES; i++) {
        Enemy *a = &game->enemies[i];
        if (!a->actor.alive) continue;
        for (int j = i + 1; j < MAX_ENEMIES; j++) {
            Enemy *b = &game->enemies[j];
            if (!b->actor.alive) continue;
            if (CheckCollisionCircles(a->actor.position, a->actor.radius, b->actor.position, b->actor.radius)) {
                bounce_actors(&a->actor, &b->actor);
            }
        }
    }
}

static void collect_eggs(Game *game, Resources *res) {
    for (int i = 0; i < MAX_EGGS; i++) {
        Egg *egg = &game->eggs[i];
        if (!egg->active) continue;
        if (CheckCollisionCircles(game->player.position, game->player.radius, egg->position, EGG_RADIUS)) {
            egg->active = false;
            game->score += 250;
            if (game->score > game->high_score) game->high_score = game->score;
            spawn_particles(game, egg->position, (Color){ 255, 245, 181, 255 }, 12);
            sound_play(res, SOUND_EGG_COLLECT);
        }
    }
}

static void resolve_hazards(Game *game, Resources *res) {
    if (CheckCollisionCircleRec(game->player.position, game->player.radius, game->lava)) {
        if (game->player.invuln_timer > 0.0f) {
            // Don't kill while invulnerable, but don't let the player sit silently
            // in the lava either — bounce out the way an enemy would.
            game->player.position.y = game->lava.y - game->player.radius;
            game->player.velocity.y = PLAYER_FLAP_VELOCITY * 0.6f;
        } else {
            sound_play(res, SOUND_LAVA);
            kill_player(game, res);
        }
    }

    for (int i = 0; i < MAX_ENEMIES; i++) {
        Enemy *enemy = &game->enemies[i];
        if (!enemy->actor.alive) continue;
        if (enemy->lava_recover_timer > 0.0f) continue;
        if (CheckCollisionCircleRec(enemy->actor.position, enemy->actor.radius, game->lava)) {
            enemy->actor.position.y = game->lava.y - enemy->actor.radius;
            enemy->actor.velocity.y = ENEMY_TUNING[enemy->type].flap_velocity * ENEMY_LAVA_BOUNCE_SCALE;
            enemy->actor.flap_cooldown = ENEMY_TUNING[enemy->type].flap_cooldown;
            enemy->lava_recover_timer = ENEMY_LAVA_RECOVER_GRACE;
        }
    }
}

static void update_playing(Game *game, Resources *res, float dt) {
    update_player(game, res, dt);
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (game->enemies[i].actor.alive) {
            update_enemy(game, &game->enemies[i], dt);
        }
    }
    update_eggs(game, res, dt);
    resolve_jousts(game, res);
    if (game->mode != GAME_MODE_PLAYING) return;
    collect_eggs(game, res);
    resolve_hazards(game, res);

    if (active_enemy_count(game) == 0 && active_egg_count(game) == 0) {
        game->score += 1000 + game->wave * 250;
        if (game->score > game->high_score) game->high_score = game->score;
        game->mode = GAME_MODE_WAVE_CLEAR;
        game->mode_timer = WAVE_CLEAR_TIME;
        spawn_particles(game, (Vector2){ WINDOW_WIDTH * 0.5f, WINDOW_HEIGHT * 0.42f }, (Color){ 139, 238, 164, 255 }, 52);
        sound_play(res, SOUND_WAVE_CLEAR);
    }
}

static void update_debug(Game *game, Resources *res) {
    if (IsKeyPressed(KEY_F1)) {
        game->debug_draw = !game->debug_draw;
    }
    if (game->mode != GAME_MODE_PLAYING) {
        return;
    }
    if (IsKeyPressed(KEY_F2)) {
        spawn_enemy(game, ENEMY_GRUNT, ENEMY_SPAWN_POINTS[GetRandomValue(0, 4)]);
    }
    if (IsKeyPressed(KEY_F3)) {
        clear_wave_entities(game);
    }
    if (IsKeyPressed(KEY_F4)) {
        game->player.invuln_timer = 0.0f;
        kill_player(game, res);
    }
}

void game_init(Game *game) {
    memset(game, 0, sizeof(*game));
    SetRandomSeed((unsigned int)GetTime());
    setup_arena(game);
    game->mode = GAME_MODE_TITLE;
    game->paused_from = GAME_MODE_PLAYING;
    game->high_score = 0;
    game->wave = 1;
    reset_actor(&game->player, (Vector2){ WINDOW_WIDTH * 0.5f, 150.0f }, PLAYER_RADIUS);
}

void game_update(Game *game, Resources *res) {
    float dt = frame_dt();
    update_debug(game, res);

    if (game->mode == GAME_MODE_TITLE) {
        update_particles(game, dt);
        if (start_pressed()) {
            start_new_game(game, res);
        }
        return;
    }

    if (game->mode == GAME_MODE_GAME_OVER) {
        update_particles(game, dt);
        if (start_pressed()) {
            start_new_game(game, res);
        }
        return;
    }

    if (game->mode == GAME_MODE_PAUSED) {
        if (pause_pressed()) {
            game->mode = game->paused_from;
            sound_play(res, SOUND_PAUSE);
        }
        return;
    }

    if (pause_pressed() && game->mode == GAME_MODE_PLAYING) {
        game->paused_from = game->mode;
        game->mode = GAME_MODE_PAUSED;
        sound_play(res, SOUND_PAUSE);
        return;
    }

    update_particles(game, dt);

    if (game->mode == GAME_MODE_PLAYING) {
        update_playing(game, res, dt);
    } else if (game->mode == GAME_MODE_WAVE_CLEAR) {
        game->mode_timer -= dt;
        if (game->mode_timer <= 0.0f) {
            game->wave++;
            game->mode = GAME_MODE_PLAYING;
            setup_wave(game);
        }
    } else if (game->mode == GAME_MODE_PLAYER_DEAD) {
        game->mode_timer -= dt;
        if (game->mode_timer <= 0.0f) {
            if (game->lives > 0) {
                game->mode = GAME_MODE_PLAYING;
                respawn_player(game);
            } else {
                game->mode = GAME_MODE_GAME_OVER;
            }
        }
    }
}

static void draw_centered_texture(Resources *res, TextureID id, Vector2 position, float scale, int facing, Color tint) {
    if (!res->textures[id].loaded) return;

    Texture2D texture = res->textures[id].texture;
    Rectangle source = { 0.0f, 0.0f, (float)texture.width, (float)texture.height };
    if (facing < 0) {
        source.width = -source.width;
    }
    Rectangle dest = {
        position.x,
        position.y,
        (float)texture.width * scale,
        (float)texture.height * scale
    };
    Vector2 origin = { dest.width * 0.5f, dest.height * 0.5f };
    DrawTexturePro(texture, source, dest, origin, 0.0f, tint);
}

static void draw_background(void) {
    ClearBackground((Color){ 12, 17, 27, 255 });
    for (int y = 80; y < WINDOW_HEIGHT - 80; y += 90) {
        Color band = (Color){ 18, 27, 42, 110 };
        DrawRectangle(0, y, WINDOW_WIDTH, 2, band);
    }
    DrawCircleGradient(WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2, 520.0f, (Color){ 24, 36, 54, 120 }, (Color){ 12, 17, 27, 0 });
}

static void draw_platform(Resources *res, Platform platform) {
    if (res->textures[TEXTURE_PLATFORM].loaded) {
        Texture2D texture = res->textures[TEXTURE_PLATFORM].texture;
        DrawTexturePro(texture,
            (Rectangle){ 0.0f, 0.0f, (float)texture.width, (float)texture.height },
            platform.bounds,
            (Vector2){ 0.0f, 0.0f },
            0.0f,
            WHITE);
    } else {
        DrawRectangleRounded(platform.bounds, 0.25f, 8, (Color){ 79, 184, 137, 255 });
        DrawRectangleRec((Rectangle){ platform.bounds.x, platform.bounds.y, platform.bounds.width, 5.0f }, (Color){ 153, 239, 165, 255 });
    }
}

static void draw_lava(Game *game, Resources *res) {
    if (res->textures[TEXTURE_LAVA].loaded) {
        Texture2D texture = res->textures[TEXTURE_LAVA].texture;
        DrawTexturePro(texture,
            (Rectangle){ 0.0f, 0.0f, (float)texture.width, (float)texture.height },
            game->lava,
            (Vector2){ 0.0f, 0.0f },
            0.0f,
            WHITE);
    } else {
        DrawRectangleRec(game->lava, (Color){ 181, 55, 49, 255 });
        DrawRectangle((int)game->lava.x, (int)game->lava.y, (int)game->lava.width, 8, (Color){ 255, 183, 80, 255 });
    }
}

static void draw_actor_fallback(Actor actor, Color body, Color mount) {
    DrawCircleV((Vector2){ actor.position.x, actor.position.y + 5.0f }, actor.radius, mount);
    DrawCircleV((Vector2){ actor.position.x, actor.position.y - 9.0f }, actor.radius * 0.55f, body);
    DrawTriangle(
        (Vector2){ actor.position.x + actor.facing * actor.radius * 1.2f, actor.position.y + 4.0f },
        (Vector2){ actor.position.x + actor.facing * actor.radius * 0.25f, actor.position.y - 8.0f },
        (Vector2){ actor.position.x + actor.facing * actor.radius * 0.25f, actor.position.y + 14.0f },
        mount);
}

static void draw_player(Game *game, Resources *res) {
    if (!game->player.alive) return;
    bool flicker = game->player.invuln_timer > 0.0f && ((int)(game->player.invuln_timer * 12.0f) % 2) == 0;
    if (flicker) return;

    TextureID id = TEXTURE_PLAYER_IDLE;
    if (game->player.flap_cooldown > PLAYER_FLAP_COOLDOWN * 0.5f) id = TEXTURE_PLAYER_FLAP_1;
    else if (game->player.velocity.y < -80.0f) id = TEXTURE_PLAYER_FLAP_2;

    if (res->textures[id].loaded) {
        draw_centered_texture(res, id, game->player.position, 3.4f, game->player.facing, WHITE);
    } else {
        draw_actor_fallback(game->player, (Color){ 240, 246, 255, 255 }, (Color){ 61, 188, 236, 255 });
    }
}

static void draw_enemy(Resources *res, Enemy *enemy) {
    if (!enemy->actor.alive) return;
    const EnemyTuning *tuning = &ENEMY_TUNING[enemy->type];
    TextureID id = tuning->texture;
    Color tint = enemy->spawn_grace_timer > 0.0f ? Fade(WHITE, 0.55f) : WHITE;
    if (res->textures[id].loaded) {
        draw_centered_texture(res, id, enemy->actor.position, 3.4f, enemy->actor.facing, tint);
    } else {
        Color color = tuning->color;
        if (enemy->spawn_grace_timer > 0.0f) color = Fade(color, 0.55f);
        draw_actor_fallback(enemy->actor, (Color){ 235, 229, 255, 255 }, color);
    }
}

static void draw_egg(Resources *res, Egg *egg) {
    if (!egg->active) return;
    TextureID id = egg->state == EGG_HATCHING ? TEXTURE_EGG_HATCHING : TEXTURE_EGG;
    Color tint = WHITE;
    if (egg->state == EGG_HATCHING && ((int)(egg->timer * 12.0f) % 2) == 0) {
        tint = (Color){ 255, 214, 125, 255 };
    }
    if (res->textures[id].loaded) {
        draw_centered_texture(res, id, egg->position, 3.0f, 1, tint);
    } else {
        DrawCircleV(egg->position, EGG_RADIUS, tint);
        DrawCircleLines((int)egg->position.x, (int)egg->position.y, EGG_RADIUS, (Color){ 115, 84, 42, 255 });
    }
}

static void draw_particles(Game *game, Resources *res) {
    for (int i = 0; i < MAX_PARTICLES; i++) {
        Particle *particle = &game->particles[i];
        if (!particle->active) continue;
        float alpha = particle->lifetime / particle->max_lifetime;
        Color tint = Fade(particle->color, alpha);
        if (res->textures[TEXTURE_SPARK].loaded) {
            draw_centered_texture(res, TEXTURE_SPARK, particle->position, particle->radius * 0.35f, 1, tint);
        } else {
            DrawCircleV(particle->position, particle->radius, tint);
        }
    }
}

static void draw_hud(Game *game) {
    DrawText(TextFormat("SCORE %06d", game->score), 28, 24, 24, RAYWHITE);
    DrawText(TextFormat("WAVE %d", game->wave), WINDOW_WIDTH / 2 - 54, 24, 24, (Color){ 178, 228, 255, 255 });
    DrawText(TextFormat("LIVES %d   HIGH %06d", game->lives, game->high_score), WINDOW_WIDTH - 330, 24, 24, RAYWHITE);
}

static void draw_debug(Game *game) {
    if (!game->debug_draw) return;
    DrawCircleLines((int)game->player.position.x, (int)game->player.position.y, game->player.radius, SKYBLUE);
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (game->enemies[i].actor.alive) {
            DrawCircleLines((int)game->enemies[i].actor.position.x, (int)game->enemies[i].actor.position.y, game->enemies[i].actor.radius, RED);
        }
    }
    for (int i = 0; i < MAX_EGGS; i++) {
        if (game->eggs[i].active) {
            DrawCircleLines((int)game->eggs[i].position.x, (int)game->eggs[i].position.y, EGG_RADIUS, YELLOW);
            DrawText(TextFormat("%.1f", game->eggs[i].timer), (int)game->eggs[i].position.x + 12, (int)game->eggs[i].position.y - 8, 12, YELLOW);
        }
    }
    for (int i = 0; i < game->platform_count; i++) {
        DrawRectangleLinesEx(game->platforms[i].bounds, 1.0f, GREEN);
    }
    DrawRectangleLinesEx(game->lava, 1.0f, ORANGE);
    DrawText(TextFormat("Enemies %d Eggs %d Mode %d", active_enemy_count(game), active_egg_count(game), game->mode), 28, 58, 18, LIGHTGRAY);
}

static void draw_overlay_center(const char *title, const char *subtitle) {
    int title_width = MeasureText(title, 64);
    int subtitle_width = MeasureText(subtitle, 24);
    DrawRectangle(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, Fade(BLACK, 0.42f));
    DrawText(title, WINDOW_WIDTH / 2 - title_width / 2, WINDOW_HEIGHT / 2 - 78, 64, RAYWHITE);
    DrawText(subtitle, WINDOW_WIDTH / 2 - subtitle_width / 2, WINDOW_HEIGHT / 2 + 4, 24, (Color){ 196, 219, 232, 255 });
}

void game_draw(Game *game, Resources *res) {
    BeginDrawing();
    draw_background();

    draw_lava(game, res);
    for (int i = 0; i < game->platform_count; i++) {
        draw_platform(res, game->platforms[i]);
    }
    for (int i = 0; i < MAX_EGGS; i++) {
        draw_egg(res, &game->eggs[i]);
    }
    for (int i = 0; i < MAX_ENEMIES; i++) {
        draw_enemy(res, &game->enemies[i]);
    }
    draw_player(game, res);
    draw_particles(game, res);

    if (game->mode != GAME_MODE_TITLE) {
        draw_hud(game);
    }
    draw_debug(game);

    if (game->mode == GAME_MODE_TITLE) {
        int title_width = MeasureText("JOUST", 86);
        DrawText("JOUST", WINDOW_WIDTH / 2 - title_width / 2, 250, 86, RAYWHITE);
        DrawText("FLAP ABOVE YOUR RIVALS", WINDOW_WIDTH / 2 - MeasureText("FLAP ABOVE YOUR RIVALS", 26) / 2, 348, 26, (Color){ 178, 228, 255, 255 });
        DrawText("ENTER TO START", WINDOW_WIDTH / 2 - MeasureText("ENTER TO START", 28) / 2, 500, 28, (Color){ 255, 231, 137, 255 });
        DrawText("A/D OR ARROWS TO STEER   SPACE/W/UP TO FLAP", WINDOW_WIDTH / 2 - MeasureText("A/D OR ARROWS TO STEER   SPACE/W/UP TO FLAP", 20) / 2, 548, 20, LIGHTGRAY);
        DrawText(TextFormat("HIGH %06d", game->high_score), WINDOW_WIDTH / 2 - MeasureText(TextFormat("HIGH %06d", game->high_score), 22) / 2, 600, 22, LIGHTGRAY);
    } else if (game->mode == GAME_MODE_PAUSED) {
        draw_overlay_center("PAUSED", "P OR START TO RESUME");
    } else if (game->mode == GAME_MODE_WAVE_CLEAR) {
        draw_overlay_center("WAVE CLEAR", TextFormat("BONUS %d", 1000 + game->wave * 250));
    } else if (game->mode == GAME_MODE_PLAYER_DEAD) {
        draw_overlay_center(game->lives > 0 ? "READY" : "GAME OVER", game->lives > 0 ? "RESPAWNING" : "NO LIVES REMAIN");
    } else if (game->mode == GAME_MODE_GAME_OVER) {
        draw_overlay_center("GAME OVER", "ENTER TO RESTART");
    }

    EndDrawing();
}

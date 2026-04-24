#include "game.h"
#include "sounds.h"

#include <math.h>

float randf(float min, float max) {
    return min + (max - min) * ((float)GetRandomValue(0, 10000) / 10000.0f);
}

float clampf(float value, float min, float max) {
    if (value < min) return min;
    if (value > max) return max;
    return value;
}

float approach(float value, float target, float delta) {
    if (value < target) return fminf(value + delta, target);
    return fmaxf(value - delta, target);
}

float world_to_screen_x(const Game *game, float world_x) {
    return wrapped_delta(game->camera_x, world_x) + WINDOW_WIDTH * 0.5f;
}

bool near_camera(const Game *game, float world_x, float margin) {
    float sx = world_to_screen_x(game, world_x);
    return sx > -margin && sx < WINDOW_WIDTH + margin;
}

float terrain_height_at(const Terrain *terrain, float x) {
    float wrapped = wrap_x(x);
    float sample_pos = wrapped / WORLD_WIDTH * (float)(TERRAIN_SAMPLES - 1);
    int index = (int)sample_pos;
    int next = (index + 1) % TERRAIN_SAMPLES;
    float t = sample_pos - (float)index;
    return terrain->height[index] * (1.0f - t) + terrain->height[next] * t;
}

void add_score(Game *game, int points) {
    game->score += points;
    if (game->score > game->high_score) game->high_score = game->score;

    if (game->score >= game->next_extra_life) {
        game->lives++;
        game->next_extra_life += EXTRA_LIFE_SCORE;
        sound_play(game, SND_EXTRA_LIFE);
    }
}

void spawn_particle(Game *game, float x, float y, float vx, float vy, float radius, float life, Color color) {
    for (int i = 0; i < MAX_PARTICLES; i++) {
        Particle *p = &game->particles[i];
        if (!p->active) {
            p->x = wrap_x(x);
            p->y = y;
            p->vx = vx;
            p->vy = vy;
            p->radius = radius;
            p->life = life;
            p->max_life = life;
            p->color = color;
            p->active = true;
            return;
        }
    }
}

void burst(Game *game, float x, float y, Color color, int count, float speed) {
    for (int i = 0; i < count; i++) {
        float angle = randf(0.0f, PI_F * 2.0f);
        float force = randf(speed * 0.25f, speed);
        spawn_particle(game, x, y, cosf(angle) * force, sinf(angle) * force, randf(2.0f, 5.0f), randf(0.35f, 0.8f), color);
    }
}

void reset_projectiles(Game *game) {
    memset(game->lasers, 0, sizeof(game->lasers));
    memset(game->bullets, 0, sizeof(game->bullets));
    memset(game->particles, 0, sizeof(game->particles));
}

void generate_terrain(Game *game) {
    for (int i = 0; i < TERRAIN_SAMPLES; i++) {
        float t = (float)i / (float)(TERRAIN_SAMPLES - 1);
        float y = TERRAIN_BASE_Y;
        y += sinf(t * PI_F * 8.0f) * 24.0f;
        y += sinf(t * PI_F * 21.0f + 0.8f) * 18.0f;
        y += sinf(t * PI_F * 43.0f + 2.7f) * 9.0f;
        game->terrain.height[i] = clampf(y, TERRAIN_BASE_Y - TERRAIN_VARIANCE, TERRAIN_BASE_Y + TERRAIN_VARIANCE);
    }
    game->terrain.height[TERRAIN_SAMPLES - 1] = game->terrain.height[0];
}

void reset_player(Game *game) {
    game->player.x = PLAYER_START_X;
    game->player.y = PLAYER_START_Y;
    game->player.vx = 0.0f;
    game->player.vy = 0.0f;
    game->player.facing = FACE_RIGHT;
    game->player.alive = true;
    game->player.fire_timer = 0.0f;
    game->player.invuln_timer = 2.0f;
    game->camera_x = game->player.x;
}

void spawn_humanoids(Game *game) {
    game->humanoid_count = MAX_HUMANOIDS;
    for (int i = 0; i < MAX_HUMANOIDS; i++) {
        Humanoid *h = &game->humanoids[i];
        h->x = wrap_x((WORLD_WIDTH / (float)MAX_HUMANOIDS) * (float)i + randf(90.0f, 270.0f));
        h->ground_y = terrain_height_at(&game->terrain, h->x) - HUMANOID_RADIUS;
        h->y = h->ground_y;
        h->vy = 0.0f;
        h->carrier = -1;
        h->state = HUMANOID_SAFE;
    }
}

int living_humanoids(const Game *game) {
    int count = 0;
    for (int i = 0; i < MAX_HUMANOIDS; i++) {
        if (game->humanoids[i].state != HUMANOID_LOST) count++;
    }
    return count;
}

int living_enemies(const Game *game) {
    int count = 0;
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (game->enemies[i].active) count++;
    }
    return count;
}

static void spawn_enemy(Game *game, EnemyKind kind, float x, float y) {
    for (int i = 0; i < MAX_ENEMIES; i++) {
        Enemy *e = &game->enemies[i];
        if (!e->active) {
            e->x = wrap_x(x);
            e->y = y;
            e->vx = randf(-60.0f, 60.0f);
            e->vy = randf(-20.0f, 20.0f);
            e->phase = randf(0.0f, PI_F * 2.0f);
            e->fire_timer = randf(0.4f, ENEMY_FIRE_COOLDOWN);
            e->target_humanoid = -1;
            e->carrying = false;
            e->kind = kind;
            e->active = true;
            game->enemy_count++;
            return;
        }
    }
}

void start_wave(Game *game) {
    memset(game->enemies, 0, sizeof(game->enemies));
    reset_projectiles(game);

    if (living_humanoids(game) == 0) {
        spawn_humanoids(game);
    } else {
        for (int i = 0; i < MAX_HUMANOIDS; i++) {
            Humanoid *h = &game->humanoids[i];
            if (h->state == HUMANOID_LOST) continue;
            h->ground_y = terrain_height_at(&game->terrain, h->x) - HUMANOID_RADIUS;
            h->y = h->ground_y;
            h->vy = 0.0f;
            h->carrier = -1;
            h->state = HUMANOID_SAFE;
        }
    }

    game->enemy_count = 0;
    int landers = 7 + game->wave * 2;
    if (landers > MAX_ENEMIES - 3) landers = MAX_ENEMIES - 3;

    for (int i = 0; i < landers; i++) {
        float x = wrap_x(randf(0.0f, WORLD_WIDTH));
        float y = randf(145.0f, 330.0f);
        spawn_enemy(game, ENEMY_LANDER, x, y);
    }

    game->baiter_timer = 24.0f;
    game->state = STATE_READY;
    game->state_timer = 1.7f;
}

void new_game(Game *game) {
    int high_score = game->high_score;
    memset(game, 0, sizeof(*game));
    game->high_score = high_score;
    game->state = STATE_TITLE;
    game->wave = 1;
    game->lives = PLAYER_LIVES;
    game->smart_bombs = PLAYER_SMART_BOMBS;
    game->next_extra_life = EXTRA_LIFE_SCORE;
    generate_terrain(game);
    reset_player(game);
    spawn_humanoids(game);
}

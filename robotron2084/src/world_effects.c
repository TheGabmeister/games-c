#include "world_internal.h"
#include "sounds.h"
#include "raymath.h"
#include <math.h>

bool world_circles_overlap(Vector2 a, float ar, Vector2 b, float br) {
    float dx = a.x - b.x;
    float dy = a.y - b.y;
    float radius = ar + br;
    return dx * dx + dy * dy <= radius * radius;
}

void world_add_float_text(Game *game, Vector2 position, int value, Color color) {
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

void world_add_particles(Game *game, Vector2 position, Color color, int count, float speed, float radius) {
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

void world_add_score(Game *game, int value) {
    game->score += value;
    if (game->score > game->high_score) {
        game->high_score = game->score;
        game->high_score_dirty = true;
    }

    while (game->score >= game->next_extra_life_score) {
        game->lives++;
        game->next_extra_life_score += EXTRA_LIFE_SCORE;
        world_add_float_text(game, (Vector2){ game->player_position.x, game->player_position.y - 28.0f }, 0, GREEN);
        sound_play(game, SOUND_EXTRA_LIFE);
    }
}

void world_update_effects(Game *game, float dt) {
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

void world_draw_effects(Game *game) {
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

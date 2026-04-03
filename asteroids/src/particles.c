#include "particles.h"

#include <math.h>

void ParticlesSpawn(GameContext *ctx, Vector2 pos, int count, float speed,
                    float sizeMin, float sizeMax, Color color)
{
    for (int i = 0; i < MAX_PARTICLES && count > 0; i++) {
        if (ctx->particles[i].active) continue;

        float angle = (float)GetRandomValue(0, 360) * DEG2RAD;
        float spd = speed * (0.3f + 0.7f * (float)GetRandomValue(0, 100) / 100.0f);

        ctx->particles[i].position = pos;
        ctx->particles[i].velocity = (Vector2){ cosf(angle) * spd, sinf(angle) * spd };
        ctx->particles[i].maxLifetime = PARTICLE_LIFETIME_MIN +
            (PARTICLE_LIFETIME_MAX - PARTICLE_LIFETIME_MIN) *
            (float)GetRandomValue(0, 100) / 100.0f;
        ctx->particles[i].lifetime = ctx->particles[i].maxLifetime;
        ctx->particles[i].size = sizeMin +
            (sizeMax - sizeMin) * (float)GetRandomValue(0, 100) / 100.0f;
        ctx->particles[i].color = color;
        ctx->particles[i].active = true;
        count--;
    }
}

void ParticlesUpdate(GameContext *ctx, float dt)
{
    for (int i = 0; i < MAX_PARTICLES; i++) {
        if (!ctx->particles[i].active) continue;

        ctx->particles[i].position.x += ctx->particles[i].velocity.x * dt;
        ctx->particles[i].position.y += ctx->particles[i].velocity.y * dt;
        ctx->particles[i].velocity.x *= 0.99f;
        ctx->particles[i].velocity.y *= 0.99f;
        ctx->particles[i].lifetime -= dt;

        if (ctx->particles[i].lifetime <= 0) ctx->particles[i].active = false;
    }
}

void ParticlesDraw(const GameContext *ctx)
{
    for (int i = 0; i < MAX_PARTICLES; i++) {
        if (!ctx->particles[i].active) continue;

        float alpha = ctx->particles[i].lifetime / ctx->particles[i].maxLifetime;
        DrawCircleV(ctx->particles[i].position, ctx->particles[i].size,
                    ColorAlpha(ctx->particles[i].color, alpha));
    }
}

#include "particles.h"
#include <math.h>

void SpawnParticles(Particle *pool, Vector2 pos, Color color, int count,
                    float minSpeed, float maxSpeed, float minLife, float maxLife,
                    float minSize, float maxSize)
{
    int spawned = 0;
    for (int i = 0; i < PARTICLE_MAX && spawned < count; i++) {
        if (!pool[i].active) {
            pool[i].active = true;
            pool[i].pos = pos;
            float angle = (float)GetRandomValue(0, 360) * DEG2RAD;
            float speed = minSpeed + (float)GetRandomValue(0, 100) / 100.0f * (maxSpeed - minSpeed);
            pool[i].vel = (Vector2){ cosf(angle) * speed, sinf(angle) * speed };
            pool[i].color = color;
            pool[i].maxLife = minLife + (float)GetRandomValue(0, 100) / 100.0f * (maxLife - minLife);
            pool[i].life = pool[i].maxLife;
            pool[i].size = minSize + (float)GetRandomValue(0, 100) / 100.0f * (maxSize - minSize);
            spawned++;
        }
    }
}

void UpdateParticles(Particle *pool, float dt)
{
    for (int i = 0; i < PARTICLE_MAX; i++) {
        if (!pool[i].active) continue;
        pool[i].pos.x += pool[i].vel.x * dt;
        pool[i].pos.y += pool[i].vel.y * dt;
        pool[i].life -= dt;
        if (pool[i].life <= 0) pool[i].active = false;
    }
}

void DrawParticles(Particle *pool)
{
    for (int i = 0; i < PARTICLE_MAX; i++) {
        if (!pool[i].active) continue;
        float alpha = pool[i].life / pool[i].maxLife;
        float sz = pool[i].size * alpha;
        DrawCircleV(pool[i].pos, sz, Fade(pool[i].color, alpha));
    }
}

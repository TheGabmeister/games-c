#include "particles.h"

void particles_spawn(Particle particles[MAX_PARTICLES], float x, float y, Color color, int count) {
    for (int i = 0; i < count; i++) {
        for (int j = 0; j < MAX_PARTICLES; j++) {
            if (!particles[j].active) {
                Particle *p = &particles[j];
                p->active = true;
                p->x = x;
                p->y = y;
                float angle = (float)GetRandomValue(0, 360) * DEG2RAD;
                float speed = (float)GetRandomValue(20, 80);
                p->vx = cosf(angle) * speed;
                p->vy = sinf(angle) * speed;
                p->life = PARTICLE_BASE_LIFE + (float)GetRandomValue(0, PARTICLE_LIFE_VARIANCE) / 100.0f;
                p->max_life = p->life;
                p->color = color;
                break;
            }
        }
    }
}

void particles_update(Particle particles[MAX_PARTICLES], float dt) {
    for (int i = 0; i < MAX_PARTICLES; i++) {
        Particle *p = &particles[i];
        if (!p->active) continue;
        p->x += p->vx * dt;
        p->y += p->vy * dt;
        p->life -= dt;
        if (p->life <= 0) p->active = false;
    }
}

void particles_draw(Particle particles[MAX_PARTICLES]) {
    for (int i = 0; i < MAX_PARTICLES; i++) {
        Particle *p = &particles[i];
        if (!p->active) continue;
        float alpha = p->life / p->max_life;
        Color c = p->color;
        c.a = (unsigned char)(alpha * 255);
        DrawCircle((int)p->x, (int)p->y, 2.0f * alpha, c);
    }
}

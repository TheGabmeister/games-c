#include "particles.h"

#include <math.h>
#include <string.h>

void particles_init(Particles *p) {
    memset(p, 0, sizeof(*p));
}

void particles_spawn_burst(Particles *p, float x, float y, int count, Color base, float speed, float lifetime) {
    for (int i = 0; i < count; i++) {
        int slot = -1;
        for (int j = 0; j < MAX_PARTICLES; j++) {
            if (!p->pool[j].active) { slot = j; break; }
        }
        if (slot < 0) return;

        float angle = (float)GetRandomValue(0, 3600) / 10.0f * DEG2RAD;
        float spd = speed * (0.3f + 0.7f * (float)GetRandomValue(0, 100) / 100.0f);

        Particle *pt = &p->pool[slot];
        pt->x = x + (float)GetRandomValue(-4, 4);
        pt->y = y + (float)GetRandomValue(-4, 4);
        pt->vx = cosf(angle) * spd;
        pt->vy = sinf(angle) * spd - 20.0f;
        pt->life = lifetime * (0.5f + 0.5f * (float)GetRandomValue(0, 100) / 100.0f);
        pt->max_life = pt->life;

        int rv = (int)base.r + GetRandomValue(-20, 20);
        int gv = (int)base.g + GetRandomValue(-20, 20);
        int bv = (int)base.b + GetRandomValue(-10, 10);
        pt->color = (Color){
            (unsigned char)(rv < 0 ? 0 : rv > 255 ? 255 : rv),
            (unsigned char)(gv < 0 ? 0 : gv > 255 ? 255 : gv),
            (unsigned char)(bv < 0 ? 0 : bv > 255 ? 255 : bv),
            255
        };
        pt->active = true;
    }
}

void particles_update(Particles *p, float dt) {
    for (int i = 0; i < MAX_PARTICLES; i++) {
        Particle *pt = &p->pool[i];
        if (!pt->active) continue;

        pt->x += pt->vx * dt;
        pt->y += pt->vy * dt;
        pt->vy += 120.0f * dt;
        pt->life -= dt;

        if (pt->life <= 0.0f) pt->active = false;
    }
}

void particles_draw(const Particles *p) {
    BeginBlendMode(BLEND_ADDITIVE);
    for (int i = 0; i < MAX_PARTICLES; i++) {
        const Particle *pt = &p->pool[i];
        if (!pt->active) continue;

        float alpha = pt->life / pt->max_life;
        Color c = pt->color;
        c.a = (unsigned char)(255.0f * alpha * alpha);

        float size = 2.0f + 2.0f * alpha;
        DrawRectangle((int)(pt->x - size * 0.5f), (int)(pt->y - size * 0.5f), (int)size, (int)size, c);
    }
    EndBlendMode();
}

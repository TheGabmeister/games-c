#include "particles.h"
#include "drawing.h"
#include <math.h>
#include <stdlib.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

void particles_clear(Particle particles[], int max)
{
    for (int i = 0; i < max; i++)
        particles[i].active = false;
}

void particles_spawn(Particle particles[], int max, vector2 pos, int count,
                     SDL_Color color, float speed, float size)
{
    int spawned = 0;
    for (int i = 0; i < max && spawned < count; i++) {
        if (particles[i].active) continue;
        Particle *p = &particles[i];
        p->active   = true;
        p->pos      = pos;
        float angle = ((float)(rand() % 360)) * (float)M_PI / 180.0f;
        float spd   = speed * (0.4f + (float)(rand() % 60) / 100.0f);
        p->vel.x    = cosf(angle) * spd;
        p->vel.y    = sinf(angle) * spd;
        p->life     = 0.4f + (float)(rand() % 40) / 100.0f;
        p->max_life = p->life;
        p->size     = size * (0.6f + (float)(rand() % 40) / 100.0f);
        p->color    = color;
        spawned++;
    }
}

void particles_update(Particle particles[], int max, float dt)
{
    for (int i = 0; i < max; i++) {
        if (!particles[i].active) continue;
        Particle *p = &particles[i];
        p->pos.x += p->vel.x * dt;
        p->pos.y += p->vel.y * dt;
        p->vel.x *= 0.98f;
        p->vel.y *= 0.98f;
        p->life  -= dt;
        if (p->life <= 0)
            p->active = false;
    }
}

void particles_draw(const Particle particles[], int max, float ox, float oy)
{
    for (int i = 0; i < max; i++) {
        if (!particles[i].active) continue;
        const Particle *p = &particles[i];
        float alpha = p->life / p->max_life;
        float sz = p->size * alpha;
        if (sz < 0.5f) continue;
        SDL_Color c = p->color;
        c.a = (Uint8)(alpha * c.a);
        draw_circle(p->pos.x + ox, p->pos.y + oy, sz, c);
    }
}

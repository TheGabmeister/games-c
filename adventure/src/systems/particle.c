#include "../platform.h"
#include "../defines.h"
#include "particle.h"
#include <math.h>
#include <stdlib.h>

#define MAX_GLOBAL_PARTICLES 512

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

typedef struct {
    float x, y, vx, vy;
    float lifetime, max_lifetime;
    SDL_Color color;
    int size;
    bool active;
} _Particle;

static _Particle _particles[MAX_GLOBAL_PARTICLES];

//==============================================================================
// Init
//==============================================================================

void particles_init(void)
{
    for (int i = 0; i < MAX_GLOBAL_PARTICLES; ++i)
    {
        _particles[i].active = false;
    }
}

//==============================================================================
// Spawn helpers
//==============================================================================

static int _find_free_slot(void)
{
    for (int i = 0; i < MAX_GLOBAL_PARTICLES; ++i)
    {
        if (!_particles[i].active)
            return i;
    }
    return -1;
}

void particles_spawn_burst(float x, float y, int count, float lifetime,
                           float speed, SDL_Color color, int size)
{
    for (int i = 0; i < count; ++i)
    {
        int slot = _find_free_slot();
        if (slot < 0) return;

        float angle = ((float)rand() / (float)RAND_MAX) * 2.0f * (float)M_PI;
        float spd = speed * (0.5f + ((float)rand() / (float)RAND_MAX) * 0.5f);

        _particles[slot].x = x;
        _particles[slot].y = y;
        _particles[slot].vx = cosf(angle) * spd;
        _particles[slot].vy = sinf(angle) * spd;
        _particles[slot].lifetime = lifetime;
        _particles[slot].max_lifetime = lifetime;
        _particles[slot].color = color;
        _particles[slot].size = size;
        _particles[slot].active = true;
    }
}

void particles_spawn_directional(float x, float y, int count, float lifetime,
                                 float speed, float dir_x, float dir_y,
                                 SDL_Color color, int size)
{
    /* Normalize direction */
    float len = sqrtf(dir_x * dir_x + dir_y * dir_y);
    if (len > 0.0001f)
    {
        dir_x /= len;
        dir_y /= len;
    }
    else
    {
        dir_x = 0.0f;
        dir_y = -1.0f;
    }

    for (int i = 0; i < count; ++i)
    {
        int slot = _find_free_slot();
        if (slot < 0) return;

        /* Add spread: random angle offset of +/- 30 degrees */
        float spread = (((float)rand() / (float)RAND_MAX) - 0.5f) * (float)(M_PI / 3.0);
        float cos_s = cosf(spread);
        float sin_s = sinf(spread);
        float vx = dir_x * cos_s - dir_y * sin_s;
        float vy = dir_x * sin_s + dir_y * cos_s;

        float spd = speed * (0.5f + ((float)rand() / (float)RAND_MAX) * 0.5f);

        _particles[slot].x = x;
        _particles[slot].y = y;
        _particles[slot].vx = vx * spd;
        _particles[slot].vy = vy * spd;
        _particles[slot].lifetime = lifetime;
        _particles[slot].max_lifetime = lifetime;
        _particles[slot].color = color;
        _particles[slot].size = size;
        _particles[slot].active = true;
    }
}

//==============================================================================
// System callback — 0-term system (no query)
//==============================================================================

void render_particles(ecs_iter_t *it)
{
    float dt = it->delta_time;
    SDL_Renderer *renderer = get_renderer();

    for (int i = 0; i < MAX_GLOBAL_PARTICLES; ++i)
    {
        _Particle *p = &_particles[i];
        if (!p->active) continue;

        /* Update */
        p->x += p->vx * dt;
        p->y += p->vy * dt;
        p->lifetime -= dt;

        if (p->lifetime <= 0.0f)
        {
            p->active = false;
            continue;
        }

        /* Render with alpha fade based on remaining lifetime */
        float ratio = p->lifetime / p->max_lifetime;
        Uint8 alpha = (Uint8)(ratio * (float)p->color.a);
        SDL_SetRenderDrawColor(renderer, p->color.r, p->color.g, p->color.b, alpha);

        SDL_FRect rect = {
            p->x - p->size * 0.5f,
            p->y - p->size * 0.5f,
            (float)p->size,
            (float)p->size
        };
        SDL_RenderFillRect(renderer, &rect);
    }
}

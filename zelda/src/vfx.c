#include "vfx.h"

static Vfx effects[MAX_VFX];
static int effect_count;

void vfx_init(void) {
    effect_count = 0;
}

void vfx_spawn(VfxType type, Vector2 pos, float radius) {
    if (effect_count >= MAX_VFX) return;
    Vfx *v = &effects[effect_count++];
    v->type = type;
    v->pos = pos;
    v->active = true;
    v->radius = radius;

    switch (type) {
        case VFX_EXPLOSION:
            v->total_frames = BOMB_EXPLOSION_FRAMES;
            v->timer = BOMB_EXPLOSION_FRAMES;
            break;
        default:
            v->total_frames = 15;
            v->timer = 15;
            break;
    }
}

void vfx_update(void) {
    for (int i = 0; i < effect_count; i++) {
        effects[i].timer--;
        if (effects[i].timer <= 0) {
            effects[i] = effects[effect_count - 1];
            effect_count--;
            i--;
        }
    }
}

static void draw_explosion(const Vfx *v) {
    float t = 1.0f - (float)v->timer / v->total_frames;
    float r = v->radius * (0.3f + 0.7f * t);
    unsigned char alpha = (unsigned char)(255 * (1.0f - t));

    DrawCircle((int)v->pos.x, (int)v->pos.y, r,
               (Color){ 255, 200, 50, alpha });
    DrawCircle((int)v->pos.x, (int)v->pos.y, r * 0.6f,
               (Color){ 255, 100, 30, alpha });
    DrawCircle((int)v->pos.x, (int)v->pos.y, r * 0.25f,
               (Color){ 255, 255, 200, alpha });
}

void vfx_draw(void) {
    for (int i = 0; i < effect_count; i++) {
        switch (effects[i].type) {
            case VFX_EXPLOSION: draw_explosion(&effects[i]); break;
            default: break;
        }
    }
}

void vfx_clear(void) {
    effect_count = 0;
}

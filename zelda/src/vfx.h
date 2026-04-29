#ifndef VFX_H
#define VFX_H

#include "raylib.h"
#include "game_config.h"

#define MAX_VFX 16

typedef enum VfxType {
    VFX_EXPLOSION = 0,
    VFX_TYPE_COUNT
} VfxType;

typedef struct Vfx {
    VfxType type;
    Vector2 pos;
    int timer;
    int total_frames;
    float radius;
    bool active;
} Vfx;

void vfx_init(void);
void vfx_spawn(VfxType type, Vector2 pos, float radius);
void vfx_update(void);
void vfx_draw(void);
void vfx_clear(void);

#endif

#include "pickup.h"
#include "textures.h"

void pickup_spawn(Pickup pickups[], int *count, PickupType type, Vector2 pos) {
    if (*count >= MAX_PICKUPS) return;
    Pickup *p = &pickups[(*count)++];
    *p = (Pickup){0};
    p->type = type;
    p->pos = pos;
    p->active = true;
    p->lifetime = PICKUP_LIFETIME;
    switch (type) {
        case PICKUP_RUPEE: p->value = 1; break;
        case PICKUP_HEART: p->value = 2; break;
        case PICKUP_BOMB:  p->value = 1; break;
        case PICKUP_ARROW: p->value = 5; break;
        default: p->value = 1; break;
    }
}

void pickups_update(Pickup pickups[], int count) {
    for (int i = 0; i < count; i++) {
        if (!pickups[i].active) continue;
        pickups[i].lifetime--;
        if (pickups[i].lifetime <= 0) {
            pickups[i].active = false;
        }
    }
}

void pickups_draw(const Pickup pickups[], int count) {
    for (int i = 0; i < count; i++) {
        const Pickup *p = &pickups[i];
        if (!p->active) continue;

        // Flash when about to despawn
        if (p->lifetime < 120 && (p->lifetime / 6) % 2 == 0) continue;

        TextureID tex_id = TEX_COUNT;
        switch (p->type) {
            case PICKUP_RUPEE: tex_id = TEX_PICKUP_RUPEE; break;
            case PICKUP_HEART: tex_id = TEX_PICKUP_HEART; break;
            case PICKUP_BOMB:  tex_id = TEX_PICKUP_BOMB;  break;
            case PICKUP_ARROW: tex_id = TEX_PICKUP_ARROW; break;
            default: break;
        }

        if (tex_id < TEX_COUNT && IsTextureValid(textures[tex_id])) {
            DrawTextureV(textures[tex_id], p->pos, WHITE);
        } else {
            int size = 24;
            int ox = (TILE_SIZE - size) / 2;
            int oy = (TILE_SIZE - size) / 2;
            Color c;
            switch (p->type) {
                case PICKUP_RUPEE: c = (Color){ 0, 200, 50, 255 }; break;
                case PICKUP_HEART: c = RED; break;
                case PICKUP_BOMB:  c = DARKGRAY; break;
                case PICKUP_ARROW: c = (Color){ 180, 160, 120, 255 }; break;
                default:           c = WHITE; break;
            }
            DrawRectangle((int)p->pos.x + ox, (int)p->pos.y + oy, size, size, c);
        }
    }
}

void pickups_clear(Pickup pickups[], int *count) {
    for (int i = 0; i < *count; i++) {
        pickups[i].active = false;
    }
    *count = 0;
}

Rectangle pickup_hitbox(const Pickup *pickup) {
    int size = 24;
    int ox = (TILE_SIZE - size) / 2;
    int oy = (TILE_SIZE - size) / 2;
    return (Rectangle){ pickup->pos.x + ox, pickup->pos.y + oy, (float)size, (float)size };
}

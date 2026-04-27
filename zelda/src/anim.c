#include "anim.h"

void anim_set(Anim *anim, const AnimDef *def) {
    anim->def = def;
    anim->timer = def->frame_duration;
    anim->current_frame = 0;
    anim->finished = false;
}

void anim_tick(Anim *anim) {
    if (!anim->def || anim->finished) return;

    anim->timer--;
    if (anim->timer <= 0) {
        anim->current_frame++;
        if (anim->current_frame >= anim->def->frame_count) {
            if (anim->def->loops) {
                anim->current_frame = 0;
            } else {
                anim->current_frame = anim->def->frame_count - 1;
                anim->finished = true;
            }
        }
        anim->timer = anim->def->frame_duration;
    }
}

int anim_frame_index(const Anim *anim) {
    if (!anim->def) return 0;
    return anim->def->first_frame + anim->current_frame;
}

#ifndef ANIM_H
#define ANIM_H

#include <stdbool.h>

typedef struct AnimDef {
    int first_frame;
    int frame_count;
    int frame_duration;
    bool loops;
} AnimDef;

typedef struct Anim {
    const AnimDef *def;
    int timer;
    int current_frame;
    bool finished;
} Anim;

void anim_set(Anim *anim, const AnimDef *def);
void anim_tick(Anim *anim);
int anim_frame_index(const Anim *anim);

#endif

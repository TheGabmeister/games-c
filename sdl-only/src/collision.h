#ifndef COLLISION_H
#define COLLISION_H

#include "platform.h"
#include <stdbool.h>

static inline bool rects_overlap(rectangle a, rectangle b)
{
    return a.x < b.x + b.w && a.x + a.w > b.x
        && a.y < b.y + b.h && a.y + a.h > b.y;
}

#endif // COLLISION_H

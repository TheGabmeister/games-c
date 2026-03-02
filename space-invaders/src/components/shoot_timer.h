#ifndef SHOOT_TIMER_H
#define SHOOT_TIMER_H

#include <flecs.h>

typedef struct {
    float time_remaining; /* seconds until next shot; negative = uninitialized */
} ShootTimer;

ECS_COMPONENT_DECLARE(ShootTimer);

#endif /* SHOOT_TIMER_H */

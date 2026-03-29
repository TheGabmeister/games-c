#ifndef RENDER_SYSTEM_H
#define RENDER_SYSTEM_H

#include <flecs.h>

void render_clear(ecs_iter_t *it);
void render_room(ecs_iter_t *it);
void render_entities(ecs_iter_t *it);
void render_colliders(ecs_iter_t *it);

#endif

#ifndef RENDER_SYSTEM_H
#define RENDER_SYSTEM_H

#include <flecs.h>

void toggle_show_colliders(void);
void render_clear(ecs_iter_t *it);
void render_present(ecs_iter_t *it);
void render_shapes(ecs_iter_t *it);
void render_colliders(ecs_iter_t *it);
void render_sprites(ecs_iter_t *it);

#endif

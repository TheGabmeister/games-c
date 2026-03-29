#ifndef FOG_SYSTEM_H
#define FOG_SYSTEM_H
#include <flecs.h>
void fog_init(void);
void fog_fini(void);
void render_fog(ecs_iter_t *it);
#endif

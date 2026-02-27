#include "systems/physics_system.h"
#include "components/transform_comp.h"
#include "components/velocity_comp.h"

static ecs_query_t *s_query;

void physics_system_init(ecs_world_t *world) {
    s_query = ecs_query(world, {
        .terms = {
            { ecs_id(Position) },
            { ecs_id(Velocity) }
        }
    });
}

void physics_system_update(ecs_world_t *world, float dt) {
    ecs_iter_t it = ecs_query_iter(world, s_query);
    while (ecs_query_next(&it)) {
        Position *p = ecs_field(&it, Position, 0);
        Velocity *v = ecs_field(&it, Velocity, 1);
        for (int i = 0; i < it.count; i++) {
            p[i].x += v[i].x * dt;
            p[i].y += v[i].y * dt;
        }
    }
}

void physics_system_shutdown(void) {
    ecs_query_fini(s_query);
    s_query = NULL;
}

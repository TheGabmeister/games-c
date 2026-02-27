#include "systems/collision_system.h"
#include "components/box_collider_comp.h"
#include "components/transform_comp.h"
#include <flecs.h>
#include "collision_system.h"

typedef struct {
    ecs_entity_t e;
    float x, y, w, h;
} ColliderBounds;

static ecs_query_t *s_proj_q;
static ecs_query_t *s_enemy_q;
static ecs_query_t *s_enemy_proj_q;
static ecs_query_t *s_player_q;



static int collect_bounds(ecs_world_t *world, ecs_query_t *q,
                           ColliderBounds *out, int max) {
    int n = 0;
    ecs_iter_t it = ecs_query_iter(world, q);
    while (ecs_query_next(&it)) {
        Position    *p = ecs_field(&it, Position,    0);
        BoxCollider *c = ecs_field(&it, BoxCollider, 1);
        for (int i = 0; i < it.count && n < max; i++) {
            out[n++] = (ColliderBounds){ it.entities[i], p[i].x, p[i].y, c[i].w, c[i].h };
        }
    }
    return n;
}

static bool aabb_overlap(const ColliderBounds *a, float bx, float by, float bw, float bh) {
    return a->x < bx + bw && a->x + a->w > bx &&
           a->y < by + bh && a->y + a->h > by;
}

    void collision_system_init(ecs_world_t * world)
    {
    }

int collision_system_update(ecs_world_t *world, ecs_entity_t *killed_out, int max_killed) {
    ColliderBounds projs[64],  eprojs[64];
    int            nprojs = collect_bounds(world, s_proj_q,       projs,  64);
    int            neprojs = collect_bounds(world, s_enemy_proj_q, eprojs, 64);

    ecs_entity_t to_delete[128];
    int          ndel = 0;

    /* Player projectiles vs enemies */
    ecs_iter_t it = ecs_query_iter(world, s_enemy_q);
    while (ecs_query_next(&it)) {
        Position    *ep = ecs_field(&it, Position,    0);
        BoxCollider *ec = ecs_field(&it, BoxCollider, 1);
        for (int i = 0; i < it.count; i++) {
            for (int j = 0; j < nprojs; j++) {
                if (aabb_overlap(&projs[j], ep[i].x, ep[i].y, ec[i].w, ec[i].h)) {
                    if (ndel < 127) { to_delete[ndel++] = it.entities[i];
                                      to_delete[ndel++] = projs[j].e; }
                    break;
                }
            }
        }
    }

    /* Enemy projectiles vs player */
    it = ecs_query_iter(world, s_player_q);
    while (ecs_query_next(&it)) {
        Position    *pp = ecs_field(&it, Position,    0);
        BoxCollider *pc = ecs_field(&it, BoxCollider, 1);
        for (int i = 0; i < it.count; i++) {
            for (int j = 0; j < neprojs; j++) {
                if (aabb_overlap(&eprojs[j], pp[i].x, pp[i].y, pc[i].w, pc[i].h)) {
                    if (ndel < 127) { to_delete[ndel++] = it.entities[i];
                                      to_delete[ndel++] = eprojs[j].e; }
                    break;
                }
            }
        }
    }

    /* Deferred deletion — guard against duplicates */
    int nkilled = 0;
    for (int i = 0; i < ndel; i++) {
        if (!ecs_is_alive(world, to_delete[i])) continue;
        ecs_delete(world, to_delete[i]);
        if (killed_out && nkilled < max_killed)
            killed_out[nkilled++] = to_delete[i];
    }
    return nkilled;
}

void collision_system_shutdown(void) {
    ecs_query_fini(s_proj_q);
    ecs_query_fini(s_enemy_q);
    ecs_query_fini(s_enemy_proj_q);
    ecs_query_fini(s_player_q);
    s_proj_q = s_enemy_q = s_enemy_proj_q = s_player_q = NULL;
}

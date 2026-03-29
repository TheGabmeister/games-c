#include "../components/transform.h"
#include "../components/collider.h"
#include "../components/collision.h"
#include "../components/room_occupant.h"
#include "../components/carried.h"
#include "../defines.h"

#include <SDL3/SDL.h>
#include "collision.h"

/*=============================================================================
 * Internal types
 *===========================================================================*/
#define MAX_COLLIDABLES 128

typedef struct {
    ecs_entity_t entity;
    float        x, y;
    ColliderType type;
    float        hw, hh;
    uint32_t     layer, mask;
    int          room_id;
} _Entry;

/*=============================================================================
 * Overlap tests
 *===========================================================================*/
static bool _rect_rect(const _Entry *a, const _Entry *b, vector2 *normal)
{
    float dx        = b->x - a->x;
    float dy        = b->y - a->y;
    float overlap_x = (a->hw + b->hw) - SDL_fabsf(dx);
    float overlap_y = (a->hh + b->hh) - SDL_fabsf(dy);

    if (overlap_x <= 0 || overlap_y <= 0) return false;

    if (overlap_x < overlap_y)
        *normal = (vector2){ (dx > 0) ? -1.0f : 1.0f, 0.0f };
    else
        *normal = (vector2){ 0.0f, (dy > 0) ? -1.0f : 1.0f };
    return true;
}

static bool _test_overlap(const _Entry *a, const _Entry *b, vector2 *normal_a)
{
    /* Only rect-rect for now */
    return _rect_rect(a, b, normal_a);
}

/*=============================================================================
 * collision_clear
 *===========================================================================*/
void collision_clear(ecs_iter_t *it)
{
    for (int i = 0; i < it->count; ++i)
        ecs_remove(it->world, it->entities[i], Collision);
}

/*=============================================================================
 * collision_detect — only entities in the same room collide
 *===========================================================================*/
void collision_detect(ecs_iter_t *it)
{
    static ecs_query_t *_q = NULL;
    if (!_q)
    {
        _q = ecs_query(it->world, {
            .terms = {
                { ecs_id(Transform),     .inout = EcsIn },
                { ecs_id(Collider),      .inout = EcsIn },
                { ecs_id(RoomOccupant),  .inout = EcsIn },
            }
        });
    }

    _Entry entries[MAX_COLLIDABLES];
    int count = 0;

    ecs_iter_t qit = ecs_query_iter(it->world, _q);
    while (ecs_query_next(&qit))
    {
        Transform    *t = ecs_field(&qit, Transform,    0);
        Collider     *c = ecs_field(&qit, Collider,     1);
        RoomOccupant *r = ecs_field(&qit, RoomOccupant, 2);

        for (int i = 0; i < qit.count; ++i)
        {
            if (count >= MAX_COLLIDABLES) break;

            /* Skip carried entities */
            if (ecs_has(it->world, qit.entities[i], Carried)) continue;

            _Entry e = {
                .entity  = qit.entities[i],
                .x       = t[i].position.x,
                .y       = t[i].position.y,
                .type    = c[i].type,
                .layer   = c[i].layer,
                .mask    = c[i].mask,
                .room_id = r[i].room_id,
            };

            if (c[i].type == COLLIDER_RECT) {
                e.hw = c[i].rect.width  * 0.5f;
                e.hh = c[i].rect.height * 0.5f;
            } else {
                e.hw = c[i].circle.radius;
                e.hh = 0.0f;
            }

            entries[count++] = e;
        }
    }

    /* Test pairs — only same room */
    for (int a = 0; a < count; ++a)
    {
        for (int b = a + 1; b < count; ++b)
        {
            if (entries[a].room_id != entries[b].room_id) continue;

            bool ab = (entries[a].mask & entries[b].layer) != 0;
            bool ba = (entries[b].mask & entries[a].layer) != 0;
            if (!ab && !ba) continue;

            vector2 normal_a;
            if (!_test_overlap(&entries[a], &entries[b], &normal_a)) continue;

            vector2 normal_b = { -normal_a.x, -normal_a.y };

            if (ab) ecs_set(it->world, entries[a].entity, Collision,
                            { .other = entries[b].entity, .normal = normal_a });
            if (ba) ecs_set(it->world, entries[b].entity, Collision,
                            { .other = entries[a].entity, .normal = normal_b });
        }
    }
}

#include "../components/transform.h"
#include "../components/collider.h"
#include "../components/collision.h"
#include "../components/velocity.h"

#include <SDL3/SDL.h>
#include "../defines.h"

#include "collision.h"

//==============================================================================

#define MAX_COLLIDABLES 64

typedef struct
{
    ecs_entity_t entity;
    float        x, y;
    ColliderType type;
    float        hw, hh;   // half-extents for rect, or radius stored in hw for circle
    uint32_t     layer, mask;
} _Entry;

//==============================================================================
// Overlap tests — return true if overlapping, and write contact normal for a
//==============================================================================

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

static bool _circle_circle(const _Entry *a, const _Entry *b, vector2 *normal)
{
    float dx   = b->x - a->x;
    float dy   = b->y - a->y;
    float dist = SDL_sqrtf(dx * dx + dy * dy);
    float rsum = a->hw + b->hw;  // radii stored in hw

    if (dist >= rsum) return false;

    if (dist > 0.0f)
        *normal = (vector2){ -dx / dist, -dy / dist };
    else
        *normal = (vector2){ 1.0f, 0.0f };

    return true;
}

static bool _rect_circle(const _Entry *rect, const _Entry *circ, vector2 *normal)
{
    float dx = circ->x - rect->x;
    float dy = circ->y - rect->y;

    // Clamp circle center to nearest point on rect
    float cx = MAX(-rect->hw, MIN(rect->hw, dx));
    float cy = MAX(-rect->hh, MIN(rect->hh, dy));

    float ex = dx - cx;
    float ey = dy - cy;
    float dist = SDL_sqrtf(ex * ex + ey * ey);

    if (dist >= circ->hw) return false;  // radius in hw

    // Normal points from circle toward rect
    if (dist > 0.0f)
        *normal = (vector2){ ex / dist, ey / dist };
    else
        *normal = (vector2){ (dx > 0) ? -1.0f : 1.0f, 0.0f };

    return true;
}

//==============================================================================

static bool _test_overlap(const _Entry *a, const _Entry *b, vector2 *normal_a)
{
    if (a->type == COLLIDER_RECT && b->type == COLLIDER_RECT)
        return _rect_rect(a, b, normal_a);

    if (a->type == COLLIDER_CIRCLE && b->type == COLLIDER_CIRCLE)
        return _circle_circle(a, b, normal_a);

    if (a->type == COLLIDER_RECT && b->type == COLLIDER_CIRCLE)
    {
        // Normal for rect-circle: from circle's perspective, flip for rect
        vector2 n;
        if (!_rect_circle(a, b, &n)) return false;
        *normal_a = (vector2){ -n.x, -n.y };
        return true;
    }

    // a is circle, b is rect
    return _rect_circle(b, a, normal_a);
}

//==============================================================================

void collision_clear(ecs_iter_t *it)
{
    for (int i = 0; i < it->count; ++i)
        ecs_remove(it->world, it->entities[i], Collision);
}

//------------------------------------------------------------------------------

void collision_detect(ecs_iter_t *it)
{
    static ecs_query_t *_q = NULL;
    if (!_q)
    {
        _q = ecs_query(it->world, {
            .terms = {
                { ecs_id(Transform), .inout = EcsIn },
                { ecs_id(Collider),  .inout = EcsIn },
            }
        });
    }

    // Collect all collidable entities
    _Entry entries[MAX_COLLIDABLES];
    int count = 0;

    ecs_iter_t qit = ecs_query_iter(it->world, _q);
    while (ecs_query_next(&qit))
    {
        Transform *t = ecs_field(&qit, Transform, 0);
        Collider  *c = ecs_field(&qit, Collider,  1);

        for (int i = 0; i < qit.count && count < MAX_COLLIDABLES; ++i)
        {
            _Entry e = {
                .entity = qit.entities[i],
                .x      = t[i].position.x,
                .y      = t[i].position.y,
                .type   = c[i].type,
                .layer  = c[i].layer,
                .mask   = c[i].mask,
            };

            if (c[i].type == COLLIDER_RECT)
            {
                e.hw = c[i].rect.width  * 0.5f;
                e.hh = c[i].rect.height * 0.5f;
            }
            else
            {
                e.hw = c[i].circle.radius;
                e.hh = 0.0f;
            }

            entries[count++] = e;
        }
    }

    // Test all pairs
    for (int a = 0; a < count; ++a)
    {
        for (int b = a + 1; b < count; ++b)
        {
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

//------------------------------------------------------------------------------

void apply_bounce(ecs_iter_t *it)
{
    Velocity  *velocity  = ecs_field(it, Velocity,  0);
    Collision *collision = ecs_field(it, Collision, 1);

    for (int i = 0; i < it->count; ++i)
    {
        vector2 n = collision[i].normal;
        if (n.x != 0.0f && velocity[i].value.x * n.x < 0.0f)
            velocity[i].value.x = -velocity[i].value.x;
        if (n.y != 0.0f && velocity[i].value.y * n.y < 0.0f)
            velocity[i].value.y = -velocity[i].value.y;
    }
}

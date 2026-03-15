#include "../components/transform.h"
#include "../components/shape.h"
#include "../components/collider.h"
#include "../components/collision.h"
#include "../components/velocity.h"

#include <SDL3/SDL.h>

#include "collision.h"

//==============================================================================

#define MAX_COLLIDABLES 64

typedef struct
{
    ecs_entity_t entity;
    float        x, y, hw, hh;
    uint32_t     layer, mask;
} _Entry;

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
                { ecs_id(Shape),     .inout = EcsIn },
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
        Shape     *s = ecs_field(&qit, Shape,     1);
        Collider  *c = ecs_field(&qit, Collider,  2);

        for (int i = 0; i < qit.count && count < MAX_COLLIDABLES; ++i)
        {
            entries[count++] = (_Entry){
                .entity = qit.entities[i],
                .x      = t[i].position.x,
                .y      = t[i].position.y,
                .hw     = s[i].rectangle.width  * 0.5f,
                .hh     = s[i].rectangle.height * 0.5f,
                .layer  = c[i].layer,
                .mask   = c[i].mask,
            };
        }
    }

    // O(n²) AABB check over all pairs
    for (int a = 0; a < count; ++a)
    {
        for (int b = a + 1; b < count; ++b)
        {
            bool ab = (entries[a].mask & entries[b].layer) != 0;
            bool ba = (entries[b].mask & entries[a].layer) != 0;
            if (!ab && !ba) continue;

            float dx        = entries[b].x - entries[a].x;
            float dy        = entries[b].y - entries[a].y;
            float overlap_x = (entries[a].hw + entries[b].hw) - SDL_fabsf(dx);
            float overlap_y = (entries[a].hh + entries[b].hh) - SDL_fabsf(dy);

            if (overlap_x <= 0 || overlap_y <= 0) continue;

            // Normal along axis of minimum penetration (points from b to a)
            vector2 normal_a, normal_b;
            if (overlap_x < overlap_y)
            {
                normal_a = (vector2){ (dx > 0) ? -1.0f : 1.0f, 0.0f };
            }
            else
            {
                normal_a = (vector2){ 0.0f, (dy > 0) ? -1.0f : 1.0f };
            }
            normal_b = (vector2){ -normal_a.x, -normal_a.y };

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
        // Reflect only if velocity is moving toward the collision (prevents sticking)
        if (n.x != 0.0f && velocity[i].value.x * n.x < 0.0f)
            velocity[i].value.x = -velocity[i].value.x;
        if (n.y != 0.0f && velocity[i].value.y * n.y < 0.0f)
            velocity[i].value.y = -velocity[i].value.y;
    }
}

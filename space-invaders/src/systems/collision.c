#include "collision.h"
#include "tags.h"
#include "../components/transform.h"
#include "../components/box_collider.h"

#define MAX_PROJECTILES 256
#define MAX_HITS        256

typedef struct {
    ecs_entity_t entity;
    float cx, cy;   /* center = Transform.position */
    float hw, hh;   /* half-extents from BoxCollider */
} ProjInfo;

static ecs_query_t *proj_query;
static ecs_query_t *enemy_query;

void collision_system_init(ecs_world_t *world)
{
    /* Terms: [Projectile(tag), Transform, BoxCollider]
     * indices: 0 (tag), 1, 2 */
    proj_query = ecs_query(world, {
        .terms = {
            { .id = Projectile },
            { .id = ecs_id(Transform) },
            { .id = ecs_id(BoxCollider) }
        }
    });

    /* Terms: [Enemy(tag), Transform, BoxCollider]
     * indices: 0 (tag), 1, 2 */
    enemy_query = ecs_query(world, {
        .terms = {
            { .id = Enemy },
            { .id = ecs_id(Transform) },
            { .id = ecs_id(BoxCollider) }
        }
    });
}

void collision_system_run(ecs_world_t *world)
{
    /* Step 1: snapshot all live projectile colliders */
    ProjInfo projs[MAX_PROJECTILES];
    bool     proj_hit[MAX_PROJECTILES];
    int      proj_count = 0;

    for (int i = 0; i < MAX_PROJECTILES; i++) proj_hit[i] = false;

    ecs_iter_t pit = ecs_query_iter(world, proj_query);
    while (ecs_query_next(&pit))
    {
        Transform   *tr = ecs_field(&pit, Transform,   1);
        BoxCollider *bc = ecs_field(&pit, BoxCollider, 2);
        for (int i = 0; i < pit.count && proj_count < MAX_PROJECTILES; i++)
        {
            projs[proj_count++] = (ProjInfo){
                .entity = pit.entities[i],
                .cx = tr[i].position[0],
                .cy = tr[i].position[1],
                .hw = bc[i].w * 0.5f,
                .hh = bc[i].h * 0.5f
            };
        }
    }

    if (proj_count == 0) return;

    /* Step 2: test each enemy AABB against every unconsumed projectile */
    ecs_entity_t del_proj[MAX_HITS];
    ecs_entity_t del_enemy[MAX_HITS];
    int          hit_count = 0;

    ecs_iter_t eit = ecs_query_iter(world, enemy_query);
    while (ecs_query_next(&eit))
    {
        Transform   *etr = ecs_field(&eit, Transform,   1);
        BoxCollider *ebc = ecs_field(&eit, BoxCollider, 2);

        for (int i = 0; i < eit.count && hit_count < MAX_HITS; i++)
        {
            float ecx = etr[i].position[0];
            float ecy = etr[i].position[1];
            float ehw = ebc[i].w * 0.5f;
            float ehh = ebc[i].h * 0.5f;

            for (int j = 0; j < proj_count; j++)
            {
                if (proj_hit[j]) continue;

                float dx = ecx - projs[j].cx;
                float dy = ecy - projs[j].cy;
                if (dx < 0.0f) dx = -dx;
                if (dy < 0.0f) dy = -dy;

                if (dx < ehw + projs[j].hw && dy < ehh + projs[j].hh)
                {
                    del_enemy[hit_count] = eit.entities[i];
                    del_proj[hit_count]  = projs[j].entity;
                    hit_count++;
                    proj_hit[j] = true;
                    break; /* one projectile destroys one enemy */
                }
            }
        }
    }

    /* Step 3: delete hit pairs outside of any query iteration */
    for (int i = 0; i < hit_count; i++)
    {
        ecs_delete(world, del_proj[i]);
        ecs_delete(world, del_enemy[i]);
    }
}

void collision_system_destroy(void)
{
    ecs_query_fini(proj_query);
    ecs_query_fini(enemy_query);
}

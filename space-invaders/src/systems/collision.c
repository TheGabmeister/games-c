#include "collision.h"
#include "tags.h"
#include "../components/transform.h"
#include "../components/box_collider.h"
#include "../components/velocity.h"
#include "../components/health.h"
#include "../managers/audio.h"
#include "../score.h"

#define MAX_PROJECTILES 256
#define MAX_HITS        256

typedef struct {
    ecs_entity_t entity;
    float cx, cy;   /* center = Transform.position */
    float hw, hh;   /* half-extents from BoxCollider */
    float vy;       /* velocity y: <0 = player projectile, >0 = enemy projectile */
} ProjInfo;

static ecs_query_t *proj_query;
static ecs_query_t *enemy_query;
static ecs_query_t *player_query;

void collision_system_init(ecs_world_t *world)
{
    /* Terms: [Projectile(tag), Transform, BoxCollider, Velocity]
     * indices: 0 (tag), 1, 2, 3 */
    proj_query = ecs_query(world, {
        .terms = {
            { .id = Projectile },
            { .id = ecs_id(Transform) },
            { .id = ecs_id(BoxCollider) },
            { .id = ecs_id(Velocity) }
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

    /* Terms: [Player(tag), Transform, BoxCollider, Health]
     * indices: 0 (tag), 1, 2, 3 */
    player_query = ecs_query(world, {
        .terms = {
            { .id = Player },
            { .id = ecs_id(Transform) },
            { .id = ecs_id(BoxCollider) },
            { .id = ecs_id(Health) }
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
        Velocity    *vel = ecs_field(&pit, Velocity,   3);
        for (int i = 0; i < pit.count && proj_count < MAX_PROJECTILES; i++)
        {
            projs[proj_count++] = (ProjInfo){
                .entity = pit.entities[i],
                .cx = tr[i].position[0],
                .cy = tr[i].position[1],
                .hw = bc[i].w * 0.5f,
                .hh = bc[i].h * 0.5f,
                .vy = vel[i].y
            };
        }
    }

    if (proj_count == 0) return;

    /* Step 2: player projectiles (vy < 0) vs enemies */
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
                if (projs[j].vy >= 0.0f) continue; /* skip enemy projectiles */

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

    /* Step 2b: enemy projectiles (vy > 0) vs player */
    ecs_entity_t del_enemy_proj[MAX_HITS];
    ecs_entity_t hit_player[MAX_HITS];
    int          player_hit_count = 0;

    ecs_iter_t plit = ecs_query_iter(world, player_query);
    while (ecs_query_next(&plit))
    {
        Transform   *ptr = ecs_field(&plit, Transform,   1);
        BoxCollider *pbc = ecs_field(&plit, BoxCollider, 2);

        for (int i = 0; i < plit.count && player_hit_count < MAX_HITS; i++)
        {
            float pcx = ptr[i].position[0];
            float pcy = ptr[i].position[1];
            float phw = pbc[i].w * 0.5f;
            float phh = pbc[i].h * 0.5f;

            for (int j = 0; j < proj_count; j++)
            {
                if (proj_hit[j]) continue;
                if (projs[j].vy <= 0.0f) continue; /* skip player projectiles */

                float dx = pcx - projs[j].cx;
                float dy = pcy - projs[j].cy;
                if (dx < 0.0f) dx = -dx;
                if (dy < 0.0f) dy = -dy;

                if (dx < phw + projs[j].hw && dy < phh + projs[j].hh)
                {
                    del_enemy_proj[player_hit_count] = projs[j].entity;
                    hit_player[player_hit_count]     = plit.entities[i];
                    player_hit_count++;
                    proj_hit[j] = true;
                    break;
                }
            }
        }
    }

    /* Step 3: delete hit pairs outside of any query iteration */
    for (int i = 0; i < hit_count; i++)
    {
        ecs_delete(world, del_proj[i]);
        ecs_delete(world, del_enemy[i]);
        score_add(10);
        audio_play_sfx(SFX_BUMP);
    }

    /* Step 3b: delete enemy projectiles and damage player */
    for (int i = 0; i < player_hit_count; i++)
    {
        ecs_delete(world, del_enemy_proj[i]);
        Health *h = ecs_get_mut(world, hit_player[i], Health);
        if (h) h->current--;
        audio_play_sfx(SFX_BUMP);
    }
}

void collision_system_destroy(void)
{
    ecs_query_fini(proj_query);
    ecs_query_fini(enemy_query);
    ecs_query_fini(player_query);
}

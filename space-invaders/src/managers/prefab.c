#include "prefab.h"
#include "asset.h"
#include "../tags.h"
#include "../components/transform.h"
#include "../components/velocity.h"
#include "../components/sprite.h"
#include "../components/box_collider.h"
#include "../components/health.h"
#include "../settings.h"

#include <SDL3/SDL.h>
#include <string.h>

static ecs_entity_t PlayerPrefab     = 0;
static ecs_entity_t EnemyPrefab      = 0;
static ecs_entity_t ProjectilePrefab = 0;

void prefab_manager_init(ecs_world_t *world)
{
    /* --- PlayerPrefab --- */
    PlayerPrefab = ecs_entity(world, {
        .name = "PlayerPrefab",
        .add  = ecs_ids(EcsPrefab)
    });

    ecs_add_id(world, PlayerPrefab, Player);

    ecs_set(world, PlayerPrefab, BoxCollider, { .w = 32.0f, .h = 32.0f });

    /* Establish component slots; values are always overridden per-instance */
    ecs_set(world, PlayerPrefab, Transform, {
        .position = {0.0f, 0.0f}, .rotation = 0.0f, .scale = {1.0f, 1.0f}
    });
    ecs_set(world, PlayerPrefab, Velocity, { .x = 0.0f, .y = 0.0f });
    ecs_set(world, PlayerPrefab, Health,   { .current = 3, .max = 3 });

    /* --- EnemyPrefab --- */
    EnemyPrefab = ecs_entity(world, {
        .name = "EnemyPrefab",
        .add  = ecs_ids(EcsPrefab)
    });

    ecs_add_id(world, EnemyPrefab, Enemy);

    ecs_set(world, EnemyPrefab, BoxCollider, { .w = 32.0f, .h = 32.0f });

    ecs_set(world, EnemyPrefab, Transform, {
        .position = {0.0f, 0.0f}, .rotation = 0.0f, .scale = {1.0f, 1.0f}
    });
    ecs_set(world, EnemyPrefab, Velocity, { .x = 0.0f, .y = 0.0f });
    ecs_set(world, EnemyPrefab, Health,   { .current = 1, .max = 1 });

    /* --- ProjectilePrefab --- */
    ProjectilePrefab = ecs_entity(world, {
        .name = "ProjectilePrefab",
        .add  = ecs_ids(EcsPrefab)
    });

    ecs_add_id(world, ProjectilePrefab, Projectile);

    ecs_set(world, ProjectilePrefab, BoxCollider, { .w = 4.0f, .h = 16.0f });
    ecs_set(world, ProjectilePrefab, Sprite, {
        .texture = NULL,
        .color   = {255, 255, 255, 255}
    });
    ecs_set(world, ProjectilePrefab, Transform, {
        .position = {0.0f, 0.0f}, .rotation = 0.0f, .scale = {1.0f, 1.0f}
    });
    ecs_set(world, ProjectilePrefab, Velocity, { .x = 0.0f, .y = -PROJECTILE_SPEED });
}

ecs_entity_t prefab_instantiate(ecs_world_t *world,
                                const char  *prefab_name,
                                cJSON       *overrides)
{
    ecs_entity_t prefab = 0;
    if (strcmp(prefab_name, "Player") == 0)
        prefab = PlayerPrefab;
    else if (strcmp(prefab_name, "Enemy") == 0)
        prefab = EnemyPrefab;
    else if (strcmp(prefab_name, "Projectile") == 0)
        prefab = ProjectilePrefab;

    if (!prefab) {
        SDL_Log("prefab_instantiate: unknown prefab '%s'", prefab_name);
        return 0;
    }

    ecs_entity_t e = ecs_new_w_pair(world, EcsIsA, prefab);

    /* Override: Sprite */
    cJSON *s = cJSON_GetObjectItemCaseSensitive(overrides, "sprite");
    if (s) {
        cJSON *tid = cJSON_GetObjectItemCaseSensitive(s, "texture_id");
        if (cJSON_IsString(tid)) {
            Sprite sp = {
                .texture = asset_manager_get(tid->valuestring),
                .color   = {255, 255, 255, 255}
            };
            ecs_set_id(world, e, ecs_id(Sprite), sizeof(Sprite), &sp);
        }
    }

    /* Override: Transform */
    cJSON *t = cJSON_GetObjectItemCaseSensitive(overrides, "transform");
    if (t) {
        Transform tr = {
            .position = {0.0f, 0.0f}, .rotation = 0.0f, .scale = {1.0f, 1.0f}
        };

        cJSON *pos = cJSON_GetObjectItemCaseSensitive(t, "position");
        if (cJSON_IsArray(pos) && cJSON_GetArraySize(pos) >= 2) {
            tr.position[0] = (float)cJSON_GetArrayItem(pos, 0)->valuedouble;
            tr.position[1] = (float)cJSON_GetArrayItem(pos, 1)->valuedouble;
        }

        cJSON *rot = cJSON_GetObjectItemCaseSensitive(t, "rotation");
        if (cJSON_IsNumber(rot))
            tr.rotation = (float)rot->valuedouble;

        cJSON *scl = cJSON_GetObjectItemCaseSensitive(t, "scale");
        if (cJSON_IsArray(scl) && cJSON_GetArraySize(scl) >= 2) {
            tr.scale[0] = (float)cJSON_GetArrayItem(scl, 0)->valuedouble;
            tr.scale[1] = (float)cJSON_GetArrayItem(scl, 1)->valuedouble;
        }

        ecs_set_id(world, e, ecs_id(Transform), sizeof(Transform), &tr);
    }

    /* Override: Velocity */
    cJSON *v = cJSON_GetObjectItemCaseSensitive(overrides, "velocity");
    if (v) {
        Velocity vel = { .x = 0.0f, .y = 0.0f };

        cJSON *vx = cJSON_GetObjectItemCaseSensitive(v, "x");
        if (cJSON_IsNumber(vx)) vel.x = (float)vx->valuedouble;

        cJSON *vy = cJSON_GetObjectItemCaseSensitive(v, "y");
        if (cJSON_IsNumber(vy)) vel.y = (float)vy->valuedouble;

        ecs_set_id(world, e, ecs_id(Velocity), sizeof(Velocity), &vel);
    }

    /* Override: Health */
    cJSON *h = cJSON_GetObjectItemCaseSensitive(overrides, "health");
    if (h) {
        Health hp = { .current = 1, .max = 1 };

        cJSON *cur = cJSON_GetObjectItemCaseSensitive(h, "current");
        if (cJSON_IsNumber(cur)) hp.current = (int)cur->valuedouble;

        cJSON *max_hp = cJSON_GetObjectItemCaseSensitive(h, "max");
        if (cJSON_IsNumber(max_hp)) hp.max = (int)max_hp->valuedouble;

        ecs_set_id(world, e, ecs_id(Health), sizeof(Health), &hp);
    }

    return e;
}

ecs_entity_t prefab_spawn_projectile(ecs_world_t *world, float x, float y, float vy)
{
    ecs_entity_t e = ecs_new_w_pair(world, EcsIsA, ProjectilePrefab);

    ecs_set(world, e, Transform, {
        .position = {x, y}, .rotation = 0.0f, .scale = {1.0f, 1.0f}
    });
    ecs_set(world, e, Velocity, { .x = 0.0f, .y = vy });

    return e;
}

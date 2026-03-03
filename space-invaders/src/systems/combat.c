#include "combat.h"
#include "tags.h"
#include "../components/transform.h"
#include "../managers/prefab.h"
#include "../event.h"
#include "../settings.h"

static ecs_query_t *player_shoot_query;
static ecs_query_t *enemy_shoot_query;

void combat_system_init(ecs_world_t *world)
{
    player_shoot_query = ecs_query(world, {
        .terms = {
            { .id = Player },
            { .id = Shooting },
            { .id = ecs_id(Transform) }
        }
    });

    enemy_shoot_query = ecs_query(world, {
        .terms = {
            { .id = Enemy },
            { .id = Shooting },
            { .id = ecs_id(Transform) }
        }
    });
}

static void process_shooters(ecs_world_t *world, ecs_query_t *query, float vy, const char *sfx_id)
{
    ecs_entity_t to_clear[64];
    int          count = 0;

    ecs_iter_t it = ecs_query_iter(world, query);
    while (ecs_query_next(&it))
    {
        Transform *transforms = ecs_field(&it, Transform, 2);
        for (int i = 0; i < it.count; i++)
        {
            prefab_spawn_projectile(world,
                transforms[i].position[0],
                transforms[i].position[1],
                vy);

            if (count < 64)
                to_clear[count++] = it.entities[i];
        }
    }

    if (count > 0 && sfx_id)
        event_queue_push(&g_events, &(GameEvent){
            .type = GAME_EVENT_PLAY_SOUND,
            .data.play_sound = { .sound_id = sfx_id }
        });

    for (int i = 0; i < count; i++)
        ecs_remove_id(world, to_clear[i], Shooting);
}

void combat_system_run(ecs_world_t *world)
{
    process_shooters(world, player_shoot_query, -PROJECTILE_SPEED, "sfx_laser1");
    process_shooters(world, enemy_shoot_query,  +PROJECTILE_SPEED, "sfx_laser2");
}

void combat_system_destroy(void)
{
    ecs_query_fini(player_shoot_query);
    ecs_query_fini(enemy_shoot_query);
}

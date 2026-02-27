#ifndef PREFAB_MANAGER_H
#define PREFAB_MANAGER_H

#include <flecs.h>
#include <cJSON.h>

/*
 * prefab_manager_init
 *
 * Defines the PlayerPrefab and EnemyPrefab entities. Must be called after
 * all components and tags are registered (ECS_COMPONENT_DEFINE + ecs_new for
 * Player/Enemy) and after load_level() has populated the asset manager.
 */
void prefab_manager_init(ecs_world_t *world);

/*
 * prefab_instantiate
 *
 * Creates an entity as an instance of the named prefab ("Player" or "Enemy")
 * via EcsIsA, then applies the per-instance component overrides from the
 * provided cJSON object.
 *
 * Expected override keys (all optional):
 *   "transform": { "position": [x, y], "rotation": r, "scale": [sx, sy] }
 *   "velocity":  { "x": vx, "y": vy }
 *   "health":    { "current": c, "max": m }
 *
 * Returns the new entity id, or 0 on failure.
 */
ecs_entity_t prefab_instantiate(ecs_world_t *world,
                                const char  *prefab_name,
                                cJSON       *overrides);

#endif /* PREFAB_MANAGER_H */

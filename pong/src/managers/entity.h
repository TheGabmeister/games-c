#ifndef ENTITY_MANAGER_H
#define ENTITY_MANAGER_H

#include <flecs.h>
#include "../platform.h"

#include "../managers/texture.h"
#include "../managers/sound.h"
#include "../managers/music.h"
#include "../components/shape.h"

ECS_TYPE_DECLARE(SceneType);
ECS_TYPE_DECLARE(LabelType);
ECS_TYPE_DECLARE(ImageType);

ECS_TAG_DECLARE(DebugTag);

void entity_manager_init(ecs_world_t *world);

ecs_entity_t entity_manager_spawn_debug(ecs_world_t *world, const char *message);
ecs_entity_t entity_manager_spawn_image(ecs_world_t *world, ecs_entity_t parent, TextureName id, float scale, vector2 position, SDL_Color tint);
ecs_entity_t entity_manager_spawn_shape(ecs_world_t *world, ecs_entity_t parent, Shape shape, vector2 position);
ecs_entity_t entity_manager_spawn_music(ecs_world_t *world, MusicName id, float volume);
ecs_entity_t entity_manager_spawn_viewport(ecs_world_t *world, ecs_entity_t parent, vector2 size, SDL_FRect dst, SDL_Color background);

#endif

#include "../platform.h"
#include <SDL3/SDL.h>
#include "../components/transform.h"
#include "../components/sprite.h"
#include "../components/shape.h"
#include "../components/audible.h"

#include "entity.h"

//==============================================================================

static void _fini(ecs_world_t *world, void *context)
{
}

//------------------------------------------------------------------------------

void entity_manager_init(ecs_world_t *world)
{
    ecs_atfini(world, _fini, NULL);

    ECS_TAG_DEFINE(world, DebugTag);
}


//------------------------------------------------------------------------------

ecs_entity_t spawn_debug(ecs_world_t *world, const char *text)
{
    ecs_entity_t entity = ecs_new(world);
    ecs_add(world, entity, DebugTag);
    return entity;
}

//------------------------------------------------------------------------------

ecs_entity_t spawn_entity(ecs_world_t *world, ecs_entity_t parent, TextureName id, float scale, vector2 position, SDL_Color tint)
{
    ecs_entity_t entity = ecs_new(world);
    SDL_Texture *texture = texture_manager_get(id);
    float tex_w, tex_h;
    SDL_GetTextureSize(texture, &tex_w, &tex_h);
    ecs_set(world, entity, Sprite, {.texture = texture, .scale = scale, .src = (SDL_FRect){0, 0, tex_w, tex_h}});
    ecs_set(world, entity, Transform, {.position = position});
    ecs_add_pair(world, entity, EcsChildOf, parent);
    return entity;
}

//------------------------------------------------------------------------------

ecs_entity_t spawn_shape(ecs_world_t *world, ecs_entity_t parent, Shape shape, vector2 position)
{
    ecs_entity_t entity = ecs_new(world);
    ecs_set(world, entity, Shape, { .type = shape.type, .color = shape.color, .rectangle = shape.rectangle });
    ecs_set(world, entity, Transform, {.position = position});
    if (parent != 0)
        ecs_add_pair(world, entity, EcsChildOf, parent);
    return entity;
}

//------------------------------------------------------------------------------

ecs_entity_t spawn_sound(ecs_world_t *world, ecs_entity_t parent, SoundName id, float volume)
{
    ecs_entity_t entity = ecs_new(world);
    MIX_Audio *sound = sound_manager_get(id);
    ecs_set(world, entity, Audible, {.sound = sound, .volume = volume});
    if (parent != 0)
        ecs_add_pair(world, entity, EcsChildOf, parent);
    return entity;
}

//------------------------------------------------------------------------------

ecs_entity_t spawn_music(ecs_world_t *world, MusicName id, float volume)
{
    ecs_entity_t entity = ecs_new(world);
    MIX_Audio *music = music_manager_get(id);
    //ecs_set(world, entity, Track, {.music = music, .volume = volume});
    return entity;
}



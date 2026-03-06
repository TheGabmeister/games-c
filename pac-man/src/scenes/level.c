#include "level.h"

#include "../systems/scene.h"

#include "../managers/entity.h"
#include "../managers/texture.h"

#include "../components/spatial.h"
#include "../components/physical.h"
#include "../components/input.h"
#include "../components/renderable.h"
#include "../components/tinted.h"

#include "../defines.h"

//==============================================================================

static ecs_entity_t _player = 0;

static const float _player_speed = 180.0f;

ecs_entity_t spawn_level(ecs_world_t *world, int value)
{
    return entity_manager_spawn_scene(world, SCENE_LEVEL, (color){255,0,0,0}, MAX_TEXTURES);
}

//------------------------------------------------------------------------------

void init_level(ecs_world_t *world, ecs_entity_t parent)
{
    vector2 position = {RASTER_WIDTH * 0.5, RASTER_HEIGHT * 0.5};
    SDL_Texture *texture = texture_manager_get(TEXTURE_SHEEP);
    float tex_w, tex_h;
    SDL_GetTextureSize(texture, &tex_w, &tex_h);

    _player = ecs_new(world);
    ecs_set(world, _player, Spatial, {.position = position, .rotation = 0.0f});
    ecs_set(world, _player, Physical, {.type = BODY_TYPE_NONE});
    ecs_set(world, _player, Renderable, {.texture = texture, .src = (SDL_FRect){0, 0, tex_w, tex_h}, .scale = 0.35f});
    ecs_set(world, _player, Tinted, {.tint = (color){255, 255, 255, 255}});
    ecs_add_pair(world, _player, EcsChildOf, parent);
}

//------------------------------------------------------------------------------

bool update_level(ecs_world_t *world, const Scene *scene, ecs_entity_t parent, const Input *input, const Time *time, const Settings *settings)
{
    if (_player != 0 && ecs_is_alive(world, _player))
    {
        Spatial *spatial = ecs_get_mut(world, _player, Spatial);
        if (spatial != NULL)
        {
            spatial->position.x += input->joystick.x * _player_speed * time->delta;
            spatial->position.y += input->joystick.y * _player_speed * time->delta;

            if (spatial->position.x < 0.0f)
                spatial->position.x = 0.0f;
            if (spatial->position.x > RASTER_WIDTH)
                spatial->position.x = RASTER_WIDTH;
            if (spatial->position.y < 0.0f)
                spatial->position.y = 0.0f;
            if (spatial->position.y > RASTER_HEIGHT)
                spatial->position.y = RASTER_HEIGHT;

            ecs_modified(world, _player, Spatial);
        }
    }

    return !input->quit;
}

//------------------------------------------------------------------------------

void fini_level(ecs_world_t *world, const Scene *scene)
{
    _player = 0;
    spawn_scene(world, SCENE_TITLE, 0);
}


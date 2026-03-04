#include <raylib.h>
#include <raymath.h>
#include <SDL3/SDL.h>
#include <engine.h>

#include "../components/spatial.h"
#include "../components/tinted.h"
#include "../components/interface.h"
#include "../components/renderable.h"
#include "../components/audible.h"
#include "../components/track.h"
#include "../components/viewport.h"
#include "../components/animated.h"
#include "../components/stateful.h"
#include "../components/transition.h"
#include "../components/aligned.h"

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

ecs_entity_t entity_manager_spawn_scene(ecs_world_t *world, SceneName id, Color color, ShaderName shader_id, TextureName texture_id)
{
  ecs_entity_t entity = ecs_new(world);
  Shader *shader = shader_manager_get(shader_id);
  SDL_Texture *texture = texture_manager_get(texture_id);
  ecs_set(world, entity, Scene, {.id = id, .color = color, .shader = shader, .texture = texture});
  ecs_set(world, entity, Stateful, {.state = STATE_CREATED, .start_time = 0.3, .run_time = 86400, .stop_time = 0.3});
  ecs_set(world, entity, Transition, {.id = TRANSITION_FADE_IN});
  return entity;
}

//------------------------------------------------------------------------------

ecs_entity_t entity_manager_spawn_label(ecs_world_t *world, ecs_entity_t parent, FontName id, const char *text, HorizontalAlignment align, VerticalAlignment valign, float size, Vector2 position, Color tint)
{
  ecs_entity_t entity = ecs_new(world);
  Font *font = font_manager_get(id);
  ecs_set(world, entity, Label, {.font = font, .text = text, .size = size});
  ecs_set(world, entity, Spatial, {.position = position});
  ecs_set(world, entity, Aligned, {.align = align, .valign = valign});
  ecs_set(world, entity, Tinted, {.tint = tint});
  if (parent != 0)
    ecs_add_pair(world, entity, EcsChildOf, parent);
  return entity;
}

//------------------------------------------------------------------------------

ecs_entity_t entity_manager_spawn_debug(ecs_world_t *world, const char *text)
{
  ecs_entity_t entity = ecs_new(world);
  ecs_set(world, entity, Label, {.text = text});
  ecs_add(world, entity, DebugTag);
  return entity;
}

//------------------------------------------------------------------------------

ecs_entity_t entity_manager_spawn_image(ecs_world_t *world, ecs_entity_t parent, TextureName id, float scale, Vector2 position, Color tint)
{
  ecs_entity_t entity = ecs_new(world);
  SDL_Texture *texture = texture_manager_get(id);
  float tex_w, tex_h;
  SDL_GetTextureSize(texture, &tex_w, &tex_h);
  ecs_set(world, entity, Renderable, {.texture = texture, .scale = scale, .src = (Rectangle){0, 0, tex_w, tex_h}});
  ecs_set(world, entity, Spatial, {.position = position});
  ecs_set(world, entity, Tinted, {.tint = tint});
  ecs_add_pair(world, entity, EcsChildOf, parent);
  return entity;
}

//------------------------------------------------------------------------------

ecs_entity_t entity_manager_spawn_sound(ecs_world_t *world, ecs_entity_t parent, SoundName id, float volume)
{
  ecs_entity_t entity = ecs_new(world);
  MIX_Audio *sound = sound_manager_get(id);
  ecs_set(world, entity, Audible, {.sound = sound, .volume = volume});
  if (parent != 0)
    ecs_add_pair(world, entity, EcsChildOf, parent);
  return entity;
}

//------------------------------------------------------------------------------

ecs_entity_t entity_manager_spawn_music(ecs_world_t *world, MusicName id, float volume)
{
  ecs_entity_t entity = ecs_new(world);
  MIX_Audio *music = music_manager_get(id);
  ecs_set(world, entity, Track, {.music = music, .volume = volume});
  return entity;
}

//------------------------------------------------------------------------------

ecs_entity_t _spawn_viewport(ecs_world_t *world, ecs_entity_t parent, Vector2 size, Rectangle dst, Color background)
{
  ecs_entity_t entity = ecs_new(world);
  SDL_Texture *raster = SDL_CreateTexture(get_renderer(), SDL_PIXELFORMAT_RGBA8888,
                                          SDL_TEXTUREACCESS_TARGET, (int)size.x, (int)size.y);
  Camera2D camera = {0};
  camera.target = Vector2Zero();
  camera.offset = Vector2Scale(size, 0.5f);
  camera.rotation = 0.0f;
  camera.zoom = 10.0f;
  Rectangle src = {0, 0, size.x, size.y};
  Vector2 origin = {dst.width * 0.5, dst.height * 0.5};
  ecs_set(world, entity, Viewport, {.raster = raster, .size = size, .background = background, .camera = camera, .src = src, .dst = dst, .origin = origin, .rotation = 0, .color = WHITE});
  ecs_add_pair(world, entity, EcsChildOf, parent);
  return entity;
}

//------------------------------------------------------------------------------

ecs_entity_t entity_manager_spawn_viewport(ecs_world_t *world, ecs_entity_t parent, Vector2 size, Rectangle dst, Color background)
{
  return _spawn_viewport(world, parent, size, dst, background);
}


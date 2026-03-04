#include "../defines.h"
#include "../helpers.h"

#include "../components/label.h"
#include "../components/spatial.h"
#include "../components/tinted.h"
#include "../components/renderable.h"
#include "../components/scene.h"
#include "../components/settings.h"
#include "../components/transition.h"
#include "../components/display.h"
#include "../components/viewport.h"
#include "../components/physical.h"
#include "../components/animated.h"
#include "../components/stateful.h"
#include "../components/aligned.h"
#include "../components/time.h"

#include "../managers/texture.h"
#include "../managers/system.h"

#include <SDL3/SDL.h>
#include <engine.h>

#include "render.h"

//==============================================================================
// Helpers
//==============================================================================

static inline rectangle _sdl_rect(rectangle r)
{
  return (rectangle){r.x, r.y, r.w, r.h};
}

static inline void _set_tint(SDL_Texture *tex, color c)
{
  SDL_SetTextureColorMod(tex, c.r, c.g, c.b);
  SDL_SetTextureAlphaMod(tex, c.a);
}

//==============================================================================

void refresh_display(ecs_iter_t *it)
{
  Display *display = ecs_singleton_get_mut(it->world, Display);

  display->window.w = GetScreenWidth();
  display->window.h = GetScreenHeight();
#ifdef MAC
  display->window.width *= GetWindowScaleDPI().x;
  display->window.height *= GetWindowScaleDPI().y;
#endif

  display->scale = MIN(display->window.w / display->raster.w, display->window.h / display->raster.h);

  display->screen = (rectangle){
      (display->window.w - (display->raster.w * display->scale)) * 0.5f,
      (display->window.h - (display->raster.h * display->scale)) * 0.5f,
      display->raster.w * display->scale,
      display->raster.h * display->scale};
}

//------------------------------------------------------------------------------

void render_scene(ecs_iter_t *it)
{
  Scene *scene = ecs_field(it, Scene, 0);
  Stateful *stateful = ecs_field(it, Stateful, 1);
  Transition *transition = ecs_field(it, Transition, 2);
  Display *display = ecs_field(it, Display, 3);
  Time *time = ecs_field(it, Time, 4);
  SDL_Renderer *renderer = get_renderer();
  SDL_Texture *playfield = texture_manager_playfield();
  SDL_SetRenderTarget(renderer, playfield);
  for (int i = 0; i < it->count; ++i)
  {
    switch (stateful[i].state)
    {
    case STATE_STARTING:
    case STATE_RUNNING:
    case STATE_STOPPING:
    {
      if (time->paused)
        display->background.a = 255;
      else
        display->background.a = (int)(255.0 * transition[i].fade);

      // TODO: scene[i].shader support requires SDL GPU shader pipeline
      if (scene[i].texture != NULL)
      {
        float w, h;
        SDL_GetTextureSize(scene[i].texture, &w, &h);
        rectangle src = {0, 0, w, h};
        rectangle dst = {0, 0, RASTER_WIDTH, RASTER_HEIGHT};
        SDL_RenderTexture(renderer, scene[i].texture, &src, &dst);
      }
      else
      {
        SDL_SetRenderDrawColor(renderer, scene[i].color.r, scene[i].color.g, scene[i].color.b, scene[i].color.a);
        SDL_RenderClear(renderer);
      }
      break;
    }
    }
  }
  SDL_SetRenderTarget(renderer, NULL);
}

//------------------------------------------------------------------------------

static inline vector2 _align(vector2 position, vector2 size, Aligned aligned)
{
  switch (aligned.align)
  {
  case ALIGN_LEFT:
    break;
  case ALIGN_CENTRE:
    position.x -= 0.5 * size.x;
    break;
  case ALIGN_RIGHT:
    position.x -= size.x;
    break;
  default:
    TraceLog(LOG_WARNING, "bad align");
  }
  switch (aligned.valign)
  {
  case VALIGN_TOP:
    break;
  case VALIGN_MIDDLE:
    position.y -= 0.5 * size.y;
    break;
  case VALIGN_BOTTOM:
    position.y -= size.y;
    break;
  default:
    TraceLog(LOG_WARNING, "bad valign");
  }
  return position;
}

//------------------------------------------------------------------------------

void render_labels(ecs_iter_t *it)
{
  Time *time = ecs_field(it, Time, 0);
  Label *label = ecs_field(it, Label, 1);
  Aligned *aligned = ecs_field(it, Aligned, 2);
  Spatial *spatial = ecs_field(it, Spatial, 3);
  Tinted *tinted = ecs_field(it, Tinted, 4);
  SDL_Renderer *renderer = get_renderer();
  SDL_Texture *playfield = texture_manager_playfield();
  SDL_SetRenderTarget(renderer, playfield);
  if (time->paused)
  {
    SDL_Texture *texture = texture_manager_get(TEXTURE_BLIP);
    rectangle src = {0, 0, 3, 3};
    rectangle dst = {0, 0, RASTER_WIDTH, RASTER_HEIGHT};
    _set_tint(texture, (color){0, 0, 0, 64});
    SDL_RenderTexture(renderer, texture, &src, &dst);
    dst.y = RASTER_HEIGHT * 0.5f - 70;
    dst.h = 120;
    _set_tint(texture, (color){255, 161, 0, 255});
    SDL_RenderTexture(renderer, texture, &src, &dst);
  }
  for (int i = 0; i < it->count; ++i)
  {
    // TODO: Font rendering still uses raylib — migrate font system separately
//    vector2 size = MeasureTextEx(*label[i].font, label[i].text, label[i].size, 0);
//    vector2 position = _align(spatial[i].position, size, aligned[i]);
//    DrawTextEx(*label[i].font, label[i].text, position, label[i].size, 0, tinted[i].tint);
  }
  SDL_SetRenderTarget(renderer, NULL);
}

//------------------------------------------------------------------------------

void render_images(ecs_iter_t *it)
{
  Renderable *renderable = ecs_field(it, Renderable, 0);
  Spatial *spatial = ecs_field(it, Spatial, 1);
  Tinted *tinted = ecs_field(it, Tinted, 2);
  SDL_Renderer *renderer = get_renderer();
  SDL_Texture *playfield = texture_manager_playfield();
  SDL_SetRenderTarget(renderer, playfield);
  for (int i = 0; i < it->count; ++i)
  {
    rectangle src = _sdl_rect(renderable[i].src);
    float w = renderable[i].scale * renderable[i].src.w;
    float h = renderable[i].scale * renderable[i].src.h;
    rectangle dst = {spatial[i].position.x - w * 0.5f, spatial[i].position.y - h * 0.5f, w, h};
    _set_tint(renderable[i].texture, tinted[i].tint);
    SDL_RenderTextureRotated(renderer, renderable[i].texture, &src, &dst, spatial[i].rotation, NULL, SDL_FLIP_NONE);
  }
  SDL_SetRenderTarget(renderer, NULL);
}

//------------------------------------------------------------------------------

static inline void _render_physical(ecs_iter_t *it)
{
  // TODO: Port to SDL — requires Camera2D transform and primitive drawing equivalents
  (void)it;
}

//------------------------------------------------------------------------------

void render_physical(ecs_iter_t *it)
{
  SDL_Renderer *renderer = get_renderer();
  ecs_iter_t vit = ecs_query_iter(it->world, system_manager_viewport_query());
  while (ecs_query_next(&vit))
  {
    Viewport *viewport = ecs_field(&vit, Viewport, 0);
    for (int i = 0; i < vit.count; ++i)
    {
      if (!viewport[i].active)
        continue;
      SDL_SetRenderTarget(renderer, viewport[i].raster);
      // TODO: BeginMode2D(viewport[i].camera) — Camera2D needs SDL transform equivalent
      _render_physical(it);
      // TODO: EndMode2D()
      SDL_SetRenderTarget(renderer, NULL);
    }
  }
}

//------------------------------------------------------------------------------

void render_viewports(ecs_iter_t *it)
{
  Viewport *viewport = ecs_field(it, Viewport, 0);
  SDL_Renderer *renderer = get_renderer();
  SDL_Texture *playfield = texture_manager_playfield();
  // TODO: Font system still uses raylib
  Font *font = font_manager_get(FONT_CLOVER);
  vector2 label_size = MeasureTextEx(*font, "Connect Controller", 48, 0);
  SDL_Texture *blip = texture_manager_get(TEXTURE_BLIP);
  SDL_SetRenderTarget(renderer, playfield);
  for (int i = 0; i < it->count; ++i)
  {
    rectangle dst_r = viewport[i].dst;
    dst_r.x += viewport[i].origin.x;
    dst_r.y += viewport[i].origin.y;
    rectangle src = _sdl_rect(viewport[i].src);
    rectangle dst = {dst_r.x - viewport[i].origin.x, dst_r.y - viewport[i].origin.y, dst_r.w, dst_r.h};
    SDL_FPoint center = {viewport[i].origin.x, viewport[i].origin.y};
    _set_tint(viewport[i].raster, viewport[i].color);
    SDL_RenderTextureRotated(renderer, viewport[i].raster, &src, &dst, viewport[i].rotation, &center, SDL_FLIP_NONE);
    if (viewport[i].active)
      continue;
    rectangle blip_src = {0, 0, 3, 3};
    rectangle blip_dst = {dst_r.x - dst_r.w / 2, dst_r.y - label_size.y / 2 - 7, dst_r.w, label_size.y + 10};
    _set_tint(blip, (Color){255, 0, 255, 255});
    SDL_RenderTexture(renderer, blip, &blip_src, &blip_dst);
    vector2 position = {dst_r.x - label_size.x / 2, dst_r.y - label_size.y / 2};
    DrawTextEx(*font, "Connect Controller", position, 48, 0, (Color){0, 255, 255, 255});
  }
  SDL_SetRenderTarget(renderer, NULL);
}

//------------------------------------------------------------------------------

void composite_display(ecs_iter_t *it)
{
  Display *display = ecs_singleton_get_mut(it->world, Display);
  SDL_Renderer *renderer = get_renderer();
  SDL_Texture *playfield = texture_manager_playfield();
  SDL_SetRenderTarget(renderer, NULL);
  SDL_SetRenderDrawColor(renderer, display->border.r, display->border.g, display->border.b, display->border.a);
  SDL_RenderClear(renderer);
  rectangle src = _sdl_rect(display->raster);
  rectangle dst = _sdl_rect(display->screen);
  _set_tint(playfield, display->background);
  SDL_RenderTexture(renderer, playfield, &src, &dst);
  SDL_RenderPresent(renderer);
}

//------------------------------------------------------------------------------

void animate(ecs_iter_t *it)
{
  Animated *animated = ecs_field(it, Animated, 0);
  Renderable *renderable = ecs_field(it, Renderable, 1);

  for (int i = 0; i < it->count; ++i)
  {
    int frame = (animated[i].frames * animated[i].time) / animated[i].duration;
    if (frame >= animated[i].frames)
    {
      if (animated[i].loop)
      {
        animated[i].time = 0;
        frame = 0;
      }
      else
      {
        ecs_delete(it->world, it->entities[i]);
        continue;
      }
    }
    animated[i].time += it->delta_time;
    float tex_w;
    SDL_GetTextureSize(renderable[i].texture, &tex_w, NULL);
    int tex_width = (int)tex_w;
    int x = (frame + animated[i].begin) * animated[i].width;
    int y = (x / tex_width) * animated[i].height;
    x %= tex_width;
    renderable[i].src = (Rectangle){x, y, animated[i].width, animated[i].height};
  }
}

//==============================================================================
// Physics debug draw — still uses raylib primitives, needs porting
//==============================================================================

#ifdef DEBUG

static void _draw_dot(cpFloat size, cpVect position, cpSpaceDebugColor color, cpDataPointer data)
{
  DrawCircleV(_to_vector(position), size * 0.5, _to_color(color));
}

//------------------------------------------------------------------------------

static void _draw_circle(cpVect position, cpFloat angle, cpFloat radius, cpSpaceDebugColor outline, cpSpaceDebugColor fill, cpDataPointer data)
{
  vector2 pos = _to_vector(position);
  vector2 offset = vector2Rotate((vector2){0, 1}, RAD2DEG * angle);
  DrawCircleV(pos, radius + 0.1, _to_color(outline));
  DrawCircleV(pos, radius - 0.1, _to_color(fill));
  DrawLineV(pos, vector2Add(pos, vector2Scale(offset, radius * 0.75)), _to_color(outline));
}

//------------------------------------------------------------------------------

static void _draw_segment(cpVect from, cpVect to, cpSpaceDebugColor color, cpDataPointer data)
{
  DrawLineV(_to_vector(from), _to_vector(to), _to_color(color));
}

//------------------------------------------------------------------------------

static void _draw_fat_segment(cpVect from, cpVect to, cpFloat radius, cpSpaceDebugColor outline, cpSpaceDebugColor fill, cpDataPointer data)
{
  DrawLineEx(_to_vector(from), _to_vector(to), 2 * radius + 0.1, _to_color(outline));
  DrawLineEx(_to_vector(from), _to_vector(to), 2 * radius - 0.1, _to_color(fill));
}

//------------------------------------------------------------------------------

static void _draw_polygon(int length, const cpVect *points, cpFloat radius, cpSpaceDebugColor outline, cpSpaceDebugColor fill, cpDataPointer data)
{
  vector2 vertices[length + 2];
  vertices[0] = _to_vector(cpCentroidForPoly(length, points));
  for (int i = 0; i < length; ++i)
    vertices[i + 1] = _to_vector(points[length - i - 1]);
  vertices[length + 1] = vertices[1];
  DrawTriangleFan(vertices, length + 2, _to_color(fill));
  for (int i = 1; i < length + 1; ++i)
    DrawLineEx(vertices[i], vertices[i + 1], 2 * radius + 0.2, _to_color(outline));
}

//------------------------------------------------------------------------------

static inline cpSpaceDebugColor RGBAColor(float r, float g, float b, float a)
{
  cpSpaceDebugColor color = {r, g, b, a};
  return color;
}

//------------------------------------------------------------------------------

static inline cpSpaceDebugColor LAColor(float l, float a)
{
  cpSpaceDebugColor color = {l, l, l, a};
  return color;
}

//------------------------------------------------------------------------------

static cpSpaceDebugColor Colors[] = {
    {0xb5 / 255.0f, 0x89 / 255.0f, 0x00 / 255.0f, 1.0f},
    {0xcb / 255.0f, 0x4b / 255.0f, 0x16 / 255.0f, 1.0f},
    {0xdc / 255.0f, 0x32 / 255.0f, 0x2f / 255.0f, 1.0f},
    {0xd3 / 255.0f, 0x36 / 255.0f, 0x82 / 255.0f, 1.0f},
    {0x6c / 255.0f, 0x71 / 255.0f, 0xc4 / 255.0f, 1.0f},
    {0x26 / 255.0f, 0x8b / 255.0f, 0xd2 / 255.0f, 1.0f},
    {0x2a / 255.0f, 0xa1 / 255.0f, 0x98 / 255.0f, 1.0f},
    {0x85 / 255.0f, 0x99 / 255.0f, 0x00 / 255.0f, 1.0f},
};

static cpSpaceDebugColor _shape_colour(cpShape *shape, cpDataPointer data)
{
  if (cpShapeGetSensor(shape))
  {
    return LAColor(1.0f, 0.1f);
  }
  else
  {
    cpBody *body = cpShapeGetBody(shape);

    if (cpBodyIsSleeping(body))
    {
      return RGBAColor(0x58 / 255.0f, 0x6e / 255.0f, 0x75 / 255.0f, 1.0f);
    }
    else if (body->sleeping.idleTime > shape->space->sleepTimeThreshold)
    {
      return RGBAColor(0x93 / 255.0f, 0xa1 / 255.0f, 0xa1 / 255.0f, 1.0f);
    }
    else
    {
      uint32_t val = (uint32_t)shape->hashid;

      // scramble the bits up using Robert Jenkins' 32 bit integer hash function
      val = (val + 0x7ed55d16) + (val << 12);
      val = (val ^ 0xc761c23c) ^ (val >> 19);
      val = (val + 0x165667b1) + (val << 5);
      val = (val + 0xd3a2646c) ^ (val << 9);
      val = (val + 0xfd7046c5) + (val << 3);
      val = (val ^ 0xb55a4f09) ^ (val >> 16);
      return Colors[val & 0x7];
    }
  }
}

//------------------------------------------------------------------------------

static cpSpaceDebugDrawOptions _options = {
    _draw_circle,
    _draw_segment,
    _draw_fat_segment,
    _draw_polygon,
    _draw_dot,
    CP_SPACE_DEBUG_DRAW_SHAPES | CP_SPACE_DEBUG_DRAW_COLLISION_POINTS,
    {0xEE / 255.0f, 0xE8 / 255.0f, 0xD5 / 255.0f, 1.0f},
    _shape_colour,
    {0.0f, 0.75f, 0.0f, 1.0f},
    {1.0f, 0.0f, 0.0f, 1.0f},
    NULL};

cpSpaceDebugDrawOptions *physics_debug_options(void)
{
  return &_options;
}

#endif

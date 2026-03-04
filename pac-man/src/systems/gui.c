#include <SDL3/SDL.h>
#include <engine.h>

#include "../defines.h"

#include "../components/input.h"
#include "../components/interface.h"
#include "../components/window.h"
#include "../components/widget.h"
#include "../components/settings.h"
#include "../components/spatial.h"

#include "../managers/texture.h"
#include "../managers/font.h"
#include "../managers/entity.h"
#include "../managers/sound.h"

#include "gui.h"

//==============================================================================

static bool _scissoring = false;
static vector2 _pointer = {0.5 * RASTER_WIDTH, 267.5};
static vector2 _mouse = {0.5 * RASTER_WIDTH, 0.5 * RASTER_HEIGHT};
static bool _mode = true;
static ecs_entity_t _hovered = 0;
static float _timer = 0;
static SDL_Renderer *_renderer = NULL;

//==============================================================================

void gui_input(ecs_iter_t *it)
{
  Interface *interface = ecs_field(it, Interface, 0);
  Input *input = ecs_field(it, Input, 1);
  Window *window = ecs_field(it, Window, 3);
  _pointer.x = 0.5 * RASTER_WIDTH;
  unsigned int max = 0;
  for (int i = 0; i < it->count; ++i)
  {
    if (window[i].max > max)
      max = window[i].max;
  }
  max -= 1;
  if (_timer > 0)
  {
    if (input->joystick.y < -0.1)
    {
      _pointer.y -= window->button_height;
      _timer = -0.15;
      _mode = false;
    }
    if (input->joystick.y > 0.1)
    {
      _pointer.y += window->button_height;
      _timer = -0.15;
      _mode = false;
    }
  }
  _timer += it->delta_time;
  float clamp_min = window->bounds.y + window->button_height * 0.5f;
  float clamp_max = window->bounds.y + window->bounds.h - window->button_height * 0.5f;
  _pointer.y = MAX(clamp_min, MIN(clamp_max, _pointer.y));
  vector2 delta = {_mouse.x - input->pointer.x, _mouse.y - input->pointer.y};
  _mouse = input->pointer;
  if (delta.x * delta.x + delta.y * delta.y > 0.1 || _mode)
  {
    _pointer = _mouse;
    SDL_ShowCursor();
    _mode = true;
  }
  else if (!_mode)
  {
    SDL_HideCursor();
  }
  nk_input_begin(interface);
  nk_input_motion(interface, _pointer.x, _pointer.y);
  nk_input_scroll(interface, (struct nk_vec2){0, input->wheel});
  nk_input_button(interface, NK_BUTTON_LEFT, _pointer.x, _pointer.y, input->select);
  nk_input_end(interface);
}

//------------------------------------------------------------------------------

static inline struct nk_rect _to_rect(rectangle rect)
{
  return nk_rect(rect.x, rect.y, rect.w, rect.h);
}

//------------------------------------------------------------------------------

static inline color _from_color(struct nk_color c)
{
  return (color){c.r, c.g, c.b, c.a};
}

//------------------------------------------------------------------------------

void gui_update(ecs_iter_t *it)
{
  Interface *interface = ecs_field(it, Interface, 0);
  Window *window = ecs_field(it, Window, 1);
  Widget *widget = ecs_field(it, Widget, 2);
  bool hover = false;
  if (nk_begin(interface, window->name, _to_rect(window->bounds), window->flags))
  {
    for (int i = 0; i < it->count; ++i)
    {
      if (i == 0 || widget[i].type == WIDGET_SEPARATOR)
      {
        int count = (i == 0) ? 1 : 0;
        for (int j = i + 1; j < it->count; ++j, ++count)
          if (widget[j].type == WIDGET_SEPARATOR)
            break;
        nk_layout_row_static(interface, window->button_height, window->bounds.w, 1);
        if (nk_widget_is_hovered(interface))
        {
          hover = true;
          _hovered = it->entities[i];
        }
      }
      if (nk_widget_is_hovered(interface))
      {
        hover = true;
        _hovered = it->entities[i];
      }
      switch (widget[i].type)
      {
      case WIDGET_LABEL:
        nk_label(interface, widget[i].name, NK_TEXT_CENTERED);
        break;
      case WIDGET_BUTTON:
        if (nk_button_label(interface, widget[i].name))
          widget[i].callback(it->world, &widget[i]);
        break;
      case WIDGET_SLIDER:
        if (nk_slider_float(interface, 0, &widget[i].value, 100, 0.1))
          widget[i].callback(it->world, &widget[i]);
        break;
      }
    }
  }
  nk_end(interface);
  if (!hover)
    _hovered = 0;
}

//------------------------------------------------------------------------------

static inline void _scissor(const struct nk_command_scissor *command)
{
  SDL_Rect clip = {command->x, command->y, command->w, command->h};
  SDL_SetRenderClipRect(_renderer, &clip);
  _scissoring = true;
}

//------------------------------------------------------------------------------

static inline void _line(const struct nk_command *command)
{
  SDL_Log("Unimplemented Nuklear Command: line");
}

//------------------------------------------------------------------------------

static inline void _curve(const struct nk_command *command)
{
  SDL_Log("Unimplemented Nuklear Command: curve");
}

//------------------------------------------------------------------------------

static inline void _rect(const struct nk_command_rect *command)
{
  color c = _from_color(command->color);
  SDL_SetRenderDrawColor(_renderer, c.r, c.g, c.b, c.a);
  SDL_FRect r = {command->x, command->y, command->w, command->h};
  SDL_RenderRect(_renderer, &r);
}

//------------------------------------------------------------------------------

static inline void _rect_filled(const struct nk_command_rect_filled *command)
{
  color c = _from_color(command->color);
  SDL_SetRenderDrawColor(_renderer, c.r, c.g, c.b, c.a);
  SDL_FRect r = {command->x, command->y, command->w, command->h};
  SDL_RenderFillRect(_renderer, &r);
}

//------------------------------------------------------------------------------

static inline void _rect_multi_color(const struct nk_command *command)
{
  SDL_Log("Unimplemented Nuklear Command: rect_multi_color");
}

//------------------------------------------------------------------------------

static inline void _circle(const struct nk_command *command)
{
  SDL_Log("Unimplemented Nuklear Command: circle");
}

//------------------------------------------------------------------------------

static inline void _circle_filled(const struct nk_command_circle_filled *command)
{
  color c = _from_color(command->color);
  SDL_SetRenderDrawColor(_renderer, c.r, c.g, c.b, c.a);
  SDL_FRect r = {command->x, command->y, command->w, command->h};
  SDL_RenderFillRect(_renderer, &r);
}

//------------------------------------------------------------------------------

static inline void _arc(const struct nk_command *command)
{
  SDL_Log("Unimplemented Nuklear Command: arc");
}

//------------------------------------------------------------------------------

static inline void _arc_filled(const struct nk_command *command)
{
  SDL_Log("Unimplemented Nuklear Command: arc_filled");
}

//------------------------------------------------------------------------------

static inline void _triangle(const struct nk_command *command)
{
  SDL_Log("Unimplemented Nuklear Command: triangle");
}

//------------------------------------------------------------------------------

static inline void _triangle_filled(const struct nk_command_triangle_filled *command)
{
  color c = _from_color(command->color);
  SDL_FColor fc = {c.r / 255.0f, c.g / 255.0f, c.b / 255.0f, c.a / 255.0f};
  SDL_Vertex vertices[3] = {
    {.position = {command->b.x, command->b.y}, .color = fc},
    {.position = {command->a.x, command->a.y}, .color = fc},
    {.position = {command->c.x, command->c.y}, .color = fc},
  };
  SDL_RenderGeometry(_renderer, NULL, vertices, 3, NULL, 0);
}

//------------------------------------------------------------------------------

static inline void _polygon(const struct nk_command *command)
{
  SDL_Log("Unimplemented Nuklear Command: polygon");
}

//------------------------------------------------------------------------------

static inline void _polygon_filled(const struct nk_command *command)
{
  SDL_Log("Unimplemented Nuklear Command: polygon_filled");
}

//------------------------------------------------------------------------------

static inline void _polyline(const struct nk_command *command)
{
  SDL_Log("Unimplemented Nuklear Command: polyline");
}

//------------------------------------------------------------------------------

static inline void _text(const struct nk_command_text *command)
{
  color fore = _from_color(command->foreground);
  color back = _from_color(command->background);
  SDL_SetRenderDrawColor(_renderer, back.r, back.g, back.b, back.a);
  SDL_FRect bg_rect = {command->x, command->y, command->w, command->h};
  SDL_RenderFillRect(_renderer, &bg_rect);
  SDL_Surface *surf = TTF_RenderText_Blended(font_manager_get(FONT_CLOVER), command->string, (size_t)command->length, (SDL_Color){fore.r, fore.g, fore.b, fore.a});
  if (surf)
  {
    SDL_Texture *tex = SDL_CreateTextureFromSurface(_renderer, surf);
    SDL_DestroySurface(surf);
    SDL_FRect dst = {command->x, command->y, command->w, command->h};
    SDL_RenderTexture(_renderer, tex, NULL, &dst);
    SDL_DestroyTexture(tex);
  }
}

//------------------------------------------------------------------------------

static inline void _image(const struct nk_command *command)
{
  SDL_Log("Unimplemented Nuklear Command: image");
}

//------------------------------------------------------------------------------

static inline void _custom(const struct nk_command *command)
{
  SDL_Log("Unimplemented Nuklear Command: custom");
}

//------------------------------------------------------------------------------

void gui_render(ecs_iter_t *it)
{
  Interface *interface = ecs_singleton_get_mut(it->world, Interface);
  const struct nk_command *command;
  _renderer = get_renderer();
  SDL_Texture *playfield = texture_manager_playfield();
  _scissoring = false;
  SDL_SetRenderTarget(_renderer, playfield);
  nk_foreach(command, interface)
  {
    switch (command->type)
    {
    case NK_COMMAND_NOP:
      break;
    case NK_COMMAND_SCISSOR:
      _scissor((const struct nk_command_scissor *)command);
      break;
    case NK_COMMAND_LINE:
      _line(command);
      break;
    case NK_COMMAND_CURVE:
      _curve(command);
      break;
    case NK_COMMAND_RECT:
      _rect((const struct nk_command_rect *)command);
      break;
    case NK_COMMAND_RECT_FILLED:
      _rect_filled((const struct nk_command_rect_filled *)command);
      break;
    case NK_COMMAND_RECT_MULTI_COLOR:
      _rect_multi_color(command);
      break;
    case NK_COMMAND_CIRCLE:
      _circle(command);
      break;
    case NK_COMMAND_CIRCLE_FILLED:
      _circle_filled((const struct nk_command_circle_filled *)command);
      break;
    case NK_COMMAND_ARC:
      _arc(command);
      break;
    case NK_COMMAND_ARC_FILLED:
      _arc_filled(command);
      break;
    case NK_COMMAND_TRIANGLE:
      _triangle(command);
      break;
    case NK_COMMAND_TRIANGLE_FILLED:
      _triangle_filled((struct nk_command_triangle_filled *)command);
      break;
    case NK_COMMAND_POLYGON:
      _polygon(command);
      break;
    case NK_COMMAND_POLYGON_FILLED:
      _polygon_filled(command);
      break;
    case NK_COMMAND_POLYLINE:
      _polyline(command);
      break;
    case NK_COMMAND_TEXT:
      _text((const struct nk_command_text *)command);
      break;
    case NK_COMMAND_IMAGE:
      _image(command);
      break;
    case NK_COMMAND_CUSTOM:
      _custom(command);
      break;
    }
  }
  if (_scissoring)
    SDL_SetRenderClipRect(_renderer, NULL);
  SDL_SetRenderTarget(_renderer, NULL);
  nk_clear(interface);
}

//------------------------------------------------------------------------------

void gui_reset(int row)
{
  _pointer.y = 267.5 + row * 100;
  _hovered = 0;
  _mode = false;
}

//------------------------------------------------------------------------------

void update_console(ecs_iter_t *it)
{
  Spatial *spatial = ecs_field(it, Spatial, 0);
  for (int i = 0; i < it->count; ++i)
  {
    spatial[i].position.y = RASTER_HEIGHT * 0.75 - (it->count - i - 1) * 90;
  }
}

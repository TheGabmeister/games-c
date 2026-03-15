#include "font.h"

//==============================================================================

static TTF_Font *_font[MAX_FONTS];

//==============================================================================

static void _fini(ecs_world_t *world, void *context)
{
  for (int i = 0; i < MAX_FONTS; ++i)
    TTF_CloseFont(_font[i]);
  TTF_Quit();
}

//------------------------------------------------------------------------------

static void _load(int id, const char *name, float ptsize)
{
  char filename[256];
  sprintf(filename, "./res/fonts/%s.ttf", name);
  _font[id] = TTF_OpenFont(filename, ptsize);
}

//------------------------------------------------------------------------------

void font_manager_init(ecs_world_t *world)
{
  TTF_Init();
  ecs_atfini(world, _fini, NULL);
  _load(FONT_CLOVER, "clover-sans", 48);
}

//------------------------------------------------------------------------------

TTF_Font *font_manager_get(FontName id)
{
  return _font[id];
}

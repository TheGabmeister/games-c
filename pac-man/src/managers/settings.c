#include <raylib.h>

#include "../components/settings.h"

#include "settings.h"

//==============================================================================

#define STORAGE_DATA_FILE "storage.data"

static unsigned int _load_storage_value(unsigned int position)
{
  int size = 0;
  unsigned char *data = LoadFileData(STORAGE_DATA_FILE, &size);
  unsigned int value = 0;
  if (data)
  {
    if (size >= (int)((position + 1) * sizeof(unsigned int)))
      value = ((unsigned int *)data)[position];
    MemFree(data);
  }
  return value;
}

static void _save_storage_value(unsigned int position, unsigned int value)
{
  int size = 0;
  unsigned char *data = LoadFileData(STORAGE_DATA_FILE, &size);
  int needed = (int)((position + 1) * sizeof(unsigned int));
  if (size < needed)
  {
    unsigned char *buf = MemAlloc(needed);
    if (!buf) { MemFree(data); return; }
    if (data) for (int i = 0; i < size; i++) buf[i] = data[i];
    for (int i = size; i < needed; i++) buf[i] = 0;
    MemFree(data);
    data = buf;
    size = needed;
  }
  ((unsigned int *)data)[position] = value;
  SaveFileData(STORAGE_DATA_FILE, data, size);
  MemFree(data);
}

//------------------------------------------------------------------------------

static bool _load_bool(Settings *settings, unsigned int slot)
{
  return (_load_storage_value(slot) == 1);
}

//------------------------------------------------------------------------------

static void _load_settings(ecs_world_t *world)
{
  Settings *settings = ecs_singleton_get_mut(world, Settings);
  settings->music = _load_bool(settings, 0);
  settings->fullscreen = _load_bool(settings, 1);
  ecs_singleton_modified(world, Settings);
}

//------------------------------------------------------------------------------

static void _save_bool(const Settings *settings, unsigned int slot, bool value)
{
  _save_storage_value(slot, value ? 1 : 0);
}

//------------------------------------------------------------------------------
static void _save_settings(ecs_world_t *world)
{
  const Settings *settings = ecs_singleton_get(world, Settings);
  _save_bool(settings, 0, settings->music);
  _save_bool(settings, 1, settings->fullscreen);
}

//------------------------------------------------------------------------------

static void _fini(ecs_world_t *world, void *context)
{
  _save_settings(world);
}

//------------------------------------------------------------------------------

void settings_manager_init(ecs_world_t *world)
{
  ecs_atfini(world, _fini, NULL);
  ecs_singleton_set(world, Settings, {.music = true, .fullscreen = false, .gamepad = -1});
  _load_settings(world);
}

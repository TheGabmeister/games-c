#include <SDL3/SDL.h>
#include <SDL3_mixer/SDL_mixer.h>


#include "music.h"

//==============================================================================

static MIX_Mixer *_mixer = NULL;
static MIX_Audio *_music[MAX_MUSIC];

//==============================================================================

static void _fini(ecs_world_t *world, void *context)
{
  for (int i = 0; i < MAX_MUSIC; ++i)
  {
    if (_music[i])
      MIX_DestroyAudio(_music[i]);
    _music[i] = NULL;
  }

  if (_mixer)
  {
    MIX_DestroyMixer(_mixer);
    _mixer = NULL;
  }

  MIX_Quit();
}

//------------------------------------------------------------------------------

static void _load(int id, const char *name)
{
  char filename[256];
  SDL_snprintf(filename, sizeof(filename), "./res/mp3/%s.mp3", name);
  _music[id] = MIX_LoadAudio(_mixer, filename, false);
  if (!_music[id])
    SDL_Log("Failed to load music '%s': %s", filename, SDL_GetError());
}

//------------------------------------------------------------------------------

void music_manager_init(ecs_world_t *world)
{
  ecs_atfini(world, _fini, NULL);

  if (!MIX_Init())
  {
    SDL_Log("MIX_Init failed: %s", SDL_GetError());
    return;
  }

  _mixer = MIX_CreateMixerDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, NULL);
  if (!_mixer)
  {
    SDL_Log("MIX_CreateMixerDevice failed for music: %s", SDL_GetError());
    MIX_Quit();
    return;
  }

  _load(MUSIC_ROCK_VOMIT, "rockvomit");
}

//------------------------------------------------------------------------------

MIX_Audio *music_manager_get(MusicName id)
{
  return (id < 0 || id >= MAX_MUSIC) ? NULL : _music[id];
}

//------------------------------------------------------------------------------

MIX_Mixer *music_manager_mixer(void)
{
  return _mixer;
}
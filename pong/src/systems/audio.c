#include <SDL3/SDL.h>
#include <SDL3/SDL_properties.h>
#include <SDL3_mixer/SDL_mixer.h>

#include "../components/audible.h"
#include "../managers/music.h"

//==============================================================================

static bool _audio_ready(void)
{
  return music_manager_mixer() != NULL;
}

//------------------------------------------------------------------------------

static float _clamp_gain(float volume)
{
  if (volume < 0.0f)
    return 0.0f;
  if (volume > 1.0f)
    return 1.0f;
  return volume;
}

//------------------------------------------------------------------------------



//------------------------------------------------------------------------------


//------------------------------------------------------------------------------

void play_sounds(ecs_iter_t *it)
{
    Audible *audible = ecs_field(it, Audible, 0);
    for (int i = 0; i < it->count; ++i)
    {
        sound_manager_play(audible[i].sound, audible[i].volume);
        ecs_delete(it->world, it->entities[i]);
    }
}

//------------------------------------------------------------------------------



//------------------------------------------------------------------------------

void play_music(ecs_iter_t *it)
{

}

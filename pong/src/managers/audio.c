#include "audio.h"

#include "../event_bus.h"

#include <SDL3_mixer/SDL_mixer.h>
#include <SDL3/SDL.h>

//==============================================================================

static MIX_Mixer *_mixer;
static MIX_Audio *_sounds[SOUND_COUNT];

//------------------------------------------------------------------------------

static void _on_play_sound(const void *data)
{
    const PlaySoundData *e = data;
    if (e->id < 0 || e->id >= SOUND_COUNT) return;
    MIX_PlayAudio(_mixer, _sounds[e->id]);
}

//==============================================================================

void audio_init(void)
{
    if (!MIX_Init())
    {
        SDL_Log("MIX_Init failed: %s", SDL_GetError());
        return;
    }

    _mixer = MIX_CreateMixerDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, NULL);
    if (!_mixer)
    {
        SDL_Log("MIX_CreateMixerDevice failed: %s", SDL_GetError());
        return;
    }

    _sounds[SOUND_BUMP] = MIX_LoadAudio(_mixer, "assets/sfx_bump.ogg", true);
    if (!_sounds[SOUND_BUMP])
        SDL_Log("Failed to load sfx_bump.ogg: %s", SDL_GetError());

    event_bus_subscribe(EVENT_PLAY_SOUND, _on_play_sound);
}

void audio_fini(void)
{
    for (int i = 0; i < SOUND_COUNT; i++)
    {
        MIX_DestroyAudio(_sounds[i]);
        _sounds[i] = NULL;
    }
    MIX_DestroyMixer(_mixer);
    _mixer = NULL;
    MIX_Quit();
}

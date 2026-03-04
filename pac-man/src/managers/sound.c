#include <SDL3/SDL.h>
#include <SDL3_mixer/SDL_mixer.h>

#include "sound.h"

static int frames = 0;

//==============================================================================

static MIX_Mixer *_mixer = NULL;
static MIX_Audio *_sounds[MAX_SOUNDS];

//==============================================================================

static void SDLCALL _destroy_track(void *userdata, MIX_Track *track)
{
    (void)userdata;
    MIX_DestroyTrack(track);
}

//==============================================================================

static void _fini(ecs_world_t *world, void *context)
{
    for (int i = 0; i < MAX_SOUNDS; ++i)
    {
        if (_sounds[i])
            MIX_DestroyAudio(_sounds[i]);
        _sounds[i] = NULL;
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
    SDL_snprintf(filename, sizeof(filename), "./res/sfx/%s.wav", name);
    _sounds[id] = MIX_LoadAudio(_mixer, filename, false);
    if (!_sounds[id])
        SDL_Log("Failed to load sound '%s': %s", filename, SDL_GetError());
}

//------------------------------------------------------------------------------

void sound_manager_init(ecs_world_t *world)
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
        SDL_Log("MIX_CreateMixerDevice failed: %s", SDL_GetError());
        MIX_Quit();
        return;
    }
    _load(SOUND_KRANZKY, "kranzky");
    _load(SOUND_SHOOT, "shoot");
    _load(SOUND_BOOM, "boom");
}

//------------------------------------------------------------------------------

MIX_Audio *sound_manager_get(SoundName id)
{
    return (id < 0 || id >= MAX_SOUNDS) ? NULL : _sounds[id];
}

//------------------------------------------------------------------------------

bool sound_manager_play(MIX_Audio *sound, float volume)
{
    if (!_mixer || !sound)
        return false;
    if (volume < 0.0f)
        volume = 0.0f;
    // frames++;
    // SDL_Log("%d",frames);
    MIX_Track *track = MIX_CreateTrack(_mixer);
    if (!track)
        return false;
    if (!MIX_SetTrackAudio(track, sound))
    {
        MIX_DestroyTrack(track);
        return false;
    }
    if (!MIX_SetTrackGain(track, volume))
    {
        MIX_DestroyTrack(track);
        return false;
    }
    if (!MIX_SetTrackStoppedCallback(track, _destroy_track, NULL))
    {
        MIX_DestroyTrack(track);
        return false;
    }
    if (!MIX_PlayTrack(track, 0))
    {
        MIX_DestroyTrack(track);
        return false;
    }
    return true;
}

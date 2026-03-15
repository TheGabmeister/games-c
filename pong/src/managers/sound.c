#include <SDL3/SDL.h>
#include <SDL3_mixer/SDL_mixer.h>

#include "sound.h"

//==============================================================================

#define MAX_VOICE_TRACKS 16

static MIX_Mixer *_mixer = NULL;
static MIX_Audio *_sounds[MAX_SOUNDS];
static MIX_Track *_tracks[MAX_VOICE_TRACKS];

//==============================================================================

static void _fini(ecs_world_t *world, void *context)
{
    for (int i = 0; i < MAX_VOICE_TRACKS; ++i)
    {
        if (_tracks[i])
            MIX_DestroyTrack(_tracks[i]);
        _tracks[i] = NULL;
    }
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
    SDL_snprintf(filename, sizeof(filename), "./assets/%s.ogg", name);
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
    _load(SOUND_BUMP, "sfx_bump");
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

    // Destroy stopped tracks to reclaim slots (safe from main thread)
    for (int i = 0; i < MAX_VOICE_TRACKS; ++i)
    {
        if (_tracks[i] && !MIX_TrackPlaying(_tracks[i]))
        {
            MIX_DestroyTrack(_tracks[i]);
            _tracks[i] = NULL;
        }
    }

    // Find a free slot
    int slot = -1;
    for (int i = 0; i < MAX_VOICE_TRACKS; ++i)
    {
        if (!_tracks[i]) { slot = i; break; }
    }
    if (slot == -1)
        return false;

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
    if (!MIX_PlayTrack(track, 0))
    {
        MIX_DestroyTrack(track);
        return false;
    }
    _tracks[slot] = track;
    return true;
}

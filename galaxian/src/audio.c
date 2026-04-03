#include "audio.h"
#include <SDL3/SDL.h>

static MIX_Mixer *mixer = NULL;
static MIX_Track *music_track = NULL;

void audio_init(void)
{
    if (!MIX_Init()) {
        SDL_Log("audio_init: MIX_Init failed: %s", SDL_GetError());
        return;
    }

    mixer = MIX_CreateMixerDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, NULL);
    if (!mixer) {
        SDL_Log("audio_init: MIX_CreateMixerDevice failed: %s", SDL_GetError());
        return;
    }

    /* Dedicated track for music so we can pause/resume/stop it */
    music_track = MIX_CreateTrack(mixer);
}

void audio_shutdown(void)
{
    if (music_track) {
        MIX_DestroyTrack(music_track);
        music_track = NULL;
    }
    if (mixer) {
        MIX_DestroyMixer(mixer);
        mixer = NULL;
    }
}

MIX_Mixer *get_mixer(void)
{
    return mixer;
}

void play_sound(MIX_Audio *audio)
{
    if (!audio || !mixer) return;
    MIX_PlayAudio(mixer, audio);
}

static void play_music_internal(MIX_Audio *audio, int loops)
{
    if (!audio || !music_track) return;

    MIX_StopTrack(music_track, 0);
    MIX_SetTrackAudio(music_track, audio);

    if (loops != 0) {
        SDL_PropertiesID props = SDL_CreateProperties();
        SDL_SetNumberProperty(props, MIX_PROP_PLAY_LOOPS_NUMBER, loops);
        MIX_PlayTrack(music_track, props);
        SDL_DestroyProperties(props);
    } else {
        MIX_PlayTrack(music_track, 0);
    }
}

void play_music(MIX_Audio *audio)
{
    play_music_internal(audio, -1);
}

void play_music_once(MIX_Audio *audio)
{
    play_music_internal(audio, 0);
}

void pause_music(void)
{
    if (music_track)
        MIX_PauseTrack(music_track);
}

void resume_music(void)
{
    if (music_track)
        MIX_ResumeTrack(music_track);
}

void stop_music(void)
{
    if (music_track)
        MIX_StopTrack(music_track, 0);
}

void set_master_volume(float gain)
{
    if (mixer)
        MIX_SetMixerGain(mixer, gain);
}

void set_music_volume(float gain)
{
    if (music_track)
        MIX_SetTrackGain(music_track, gain);
}

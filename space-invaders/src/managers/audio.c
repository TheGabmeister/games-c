#include "audio.h"
#include <SDL3_mixer/SDL_mixer.h>
#include <SDL3/SDL.h>
#include <string.h>

#define AUDIO_SFX_MAX   16
#define AUDIO_MUSIC_MAX  4

typedef struct { char id[64]; MIX_Audio *audio; } SfxEntry;
typedef struct { char id[64]; MIX_Audio *audio; } MusicEntry;

static MIX_Mixer  *mixer        = NULL;
static MIX_Track  *music_track  = NULL;
static SfxEntry    sfx_entries[AUDIO_SFX_MAX];
static int         sfx_count    = 0;
static MusicEntry  music_entries[AUDIO_MUSIC_MAX];
static int         music_count  = 0;

void audio_manager_init(void)
{
    MIX_Init();
    mixer       = MIX_CreateMixerDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, NULL);
    music_track = MIX_CreateTrack(mixer);
}

void audio_manager_destroy(void)
{
    audio_stop_music();
    for (int i = 0; i < sfx_count; i++)
        MIX_DestroyAudio(sfx_entries[i].audio);
    sfx_count = 0;
    for (int i = 0; i < music_count; i++)
        MIX_DestroyAudio(music_entries[i].audio);
    music_count = 0;
    MIX_DestroyMixer(mixer); /* also destroys music_track */
    mixer       = NULL;
    music_track = NULL;
    MIX_Quit();
}

/* IDs prefixed with "music_" are loaded as streamed Music; all others as Sound. */
void audio_manager_add(const char *id, const char *file)
{
    if (strncmp(id, "music_", 6) == 0)
    {
        if (music_count >= AUDIO_MUSIC_MAX) {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "audio_manager_add: music cache full, cannot add '%s'", id);
            return;
        }
        MusicEntry *e = &music_entries[music_count++];
        strncpy_s(e->id, sizeof(e->id), id, _TRUNCATE);
        e->audio = MIX_LoadAudio(mixer, file, false); /* stream from disk */
    }
    else
    {
        if (sfx_count >= AUDIO_SFX_MAX) {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "audio_manager_add: sfx cache full, cannot add '%s'", id);
            return;
        }
        SfxEntry *e = &sfx_entries[sfx_count++];
        strncpy_s(e->id, sizeof(e->id), id, _TRUNCATE);
        e->audio = MIX_LoadAudio(mixer, file, true); /* fully pre-decode into memory */
    }
}

void audio_play_sfx(const char *id)
{
    for (int i = 0; i < sfx_count; i++) {
        if (strcmp(sfx_entries[i].id, id) == 0) {
            MIX_PlayAudio(mixer, sfx_entries[i].audio); /* fire-and-forget */
            return;
        }
    }
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "audio_play_sfx: unknown id '%s'", id);
}

void audio_play_music(const char *id)
{
    for (int i = 0; i < music_count; i++) {
        if (strcmp(music_entries[i].id, id) == 0) {
            MIX_StopTrack(music_track, 0);
            MIX_SetTrackAudio(music_track, music_entries[i].audio);
            SDL_PropertiesID props = SDL_CreateProperties();
            SDL_SetNumberProperty(props, MIX_PROP_PLAY_LOOPS_NUMBER, -1);
            MIX_PlayTrack(music_track, props);
            SDL_DestroyProperties(props);
            return;
        }
    }
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "audio_play_music: unknown id '%s'", id);
}

void audio_stop_music(void)
{
    if (music_track)
        MIX_StopTrack(music_track, 0);
}

void audio_update(void)
{
    /* MIX_Mixer runs on a background thread; nothing to do here. */
}

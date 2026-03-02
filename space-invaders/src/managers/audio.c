#include "audio.h"
#include <raudio.h>
#include <string.h>
#include <SDL3/SDL.h>

#define AUDIO_SFX_MAX   16
#define AUDIO_MUSIC_MAX  4

typedef struct { char id[64]; Sound sound; } SfxEntry;
typedef struct { char id[64]; Music music; } MusicEntry;

static SfxEntry   sfx_entries[AUDIO_SFX_MAX];
static int        sfx_count   = 0;
static MusicEntry music_entries[AUDIO_MUSIC_MAX];
static int        music_count  = 0;
static Music     *active_music = NULL;

void audio_manager_init(void)
{
    InitAudioDevice();
}

void audio_manager_destroy(void)
{
    audio_stop_music();
    for (int i = 0; i < sfx_count; i++)
        UnloadSound(sfx_entries[i].sound);
    sfx_count = 0;
    for (int i = 0; i < music_count; i++)
        UnloadMusicStream(music_entries[i].music);
    music_count = 0;
    CloseAudioDevice();
}

/* IDs prefixed with "music-" are loaded as streamed Music; all others as Sound. */
void audio_manager_add(const char *id, const char *file)
{
    if (strncmp(id, "music-", 6) == 0)
    {
        if (music_count >= AUDIO_MUSIC_MAX) {
            SDL_Log("audio_manager_add: music cache full, cannot add '%s'", id);
            return;
        }
        MusicEntry *e = &music_entries[music_count++];
        strncpy(e->id, id, sizeof(e->id) - 1);
        e->id[sizeof(e->id) - 1] = '\0';
        e->music = LoadMusicStream(file);
    }
    else
    {
        if (sfx_count >= AUDIO_SFX_MAX) {
            SDL_Log("audio_manager_add: sfx cache full, cannot add '%s'", id);
            return;
        }
        SfxEntry *e = &sfx_entries[sfx_count++];
        strncpy(e->id, id, sizeof(e->id) - 1);
        e->id[sizeof(e->id) - 1] = '\0';
        e->sound = LoadSound(file);
    }
}

void audio_play_sfx(const char *id)
{
    for (int i = 0; i < sfx_count; i++) {
        if (strcmp(sfx_entries[i].id, id) == 0) {
            PlaySound(sfx_entries[i].sound);
            return;
        }
    }
    SDL_Log("audio_play_sfx: unknown id '%s'", id);
}

void audio_play_music(const char *id)
{
    for (int i = 0; i < music_count; i++) {
        if (strcmp(music_entries[i].id, id) == 0) {
            active_music = &music_entries[i].music;
            PlayMusicStream(*active_music);
            return;
        }
    }
    SDL_Log("audio_play_music: unknown id '%s'", id);
}

void audio_stop_music(void)
{
    if (active_music) {
        StopMusicStream(*active_music);
        active_music = NULL;
    }
}

void audio_update(void)
{
    if (active_music)
        UpdateMusicStream(*active_music);
}

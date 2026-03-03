#include "audio.h"
#include <SDL3_mixer/SDL_mixer.h>
#include <SDL3/SDL.h>
#include <string.h>

#define AUDIO_SFX_MAX   16
#define AUDIO_MUSIC_MAX  4

typedef struct { char id[64]; Mix_Chunk *chunk; } SfxEntry;
typedef struct { char id[64]; Mix_Music *music; } MusicEntry;

static SfxEntry   sfx_entries[AUDIO_SFX_MAX];
static int        sfx_count   = 0;
static MusicEntry music_entries[AUDIO_MUSIC_MAX];
static int        music_count  = 0;

void audio_manager_init(void)
{
    Mix_OpenAudio(0, NULL);
}

void audio_manager_destroy(void)
{
    audio_stop_music();
    for (int i = 0; i < sfx_count; i++)
        Mix_FreeChunk(sfx_entries[i].chunk);
    sfx_count = 0;
    for (int i = 0; i < music_count; i++)
        Mix_FreeMusic(music_entries[i].music);
    music_count = 0;
    Mix_CloseAudio();
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
        strncpy(e->id, id, sizeof(e->id) - 1);
        e->id[sizeof(e->id) - 1] = '\0';
        e->music = Mix_LoadMUS(file);
    }
    else
    {
        if (sfx_count >= AUDIO_SFX_MAX) {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "audio_manager_add: sfx cache full, cannot add '%s'", id);
            return;
        }
        SfxEntry *e = &sfx_entries[sfx_count++];
        strncpy(e->id, id, sizeof(e->id) - 1);
        e->id[sizeof(e->id) - 1] = '\0';
        e->chunk = Mix_LoadWAV(file);
    }
}

void audio_play_sfx(const char *id)
{
    for (int i = 0; i < sfx_count; i++) {
        if (strcmp(sfx_entries[i].id, id) == 0) {
            Mix_PlayChannel(-1, sfx_entries[i].chunk, 0);
            return;
        }
    }
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "audio_play_sfx: unknown id '%s'", id);
}

void audio_play_music(const char *id)
{
    for (int i = 0; i < music_count; i++) {
        if (strcmp(music_entries[i].id, id) == 0) {
            Mix_PlayMusic(music_entries[i].music, -1);
            return;
        }
    }
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "audio_play_music: unknown id '%s'", id);
}

void audio_stop_music(void)
{
    Mix_HaltMusic();
}

void audio_update(void)
{
    /* SDL_mixer streams music on a background thread; nothing to do here. */
}

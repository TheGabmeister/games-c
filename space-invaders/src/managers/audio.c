#include "audio.h"
#include <raudio.h>

static Sound sfx[SFX_COUNT];

static const char *sfx_paths[SFX_COUNT] = {
    "assets/sfx_laser1.ogg",  /* SFX_LASER */
    "assets/sfx_bump.ogg",    /* SFX_BUMP  */
};

void audio_manager_init(void)
{
    InitAudioDevice();
    for (int i = 0; i < SFX_COUNT; i++)
        sfx[i] = LoadSound(sfx_paths[i]);
}

void audio_manager_destroy(void)
{
    for (int i = 0; i < SFX_COUNT; i++)
        UnloadSound(sfx[i]);
    CloseAudioDevice();
}

void audio_play_sfx(SfxId id)
{
    if (id < SFX_COUNT)
        PlaySound(sfx[id]);
}

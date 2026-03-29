#include "audio.h"
#include "../event_bus.h"

#include <SDL3_mixer/SDL_mixer.h>
#include <SDL3/SDL.h>

/*=============================================================================
 * State
 *===========================================================================*/
static MIX_Mixer *_mixer = NULL;
static MIX_Audio *_sounds[SOUND_COUNT] = {0};

/*=============================================================================
 * Event callback
 *===========================================================================*/
static void _on_play_sound(const void *data)
{
    const PlaySoundData *e = data;
    if (!_mixer || !e || e->id < 0 || e->id >= SOUND_COUNT) return;
    if (!_sounds[e->id]) return;
    MIX_PlayAudio(_mixer, _sounds[e->id]);
}

/*=============================================================================
 * Init / Fini
 *===========================================================================*/
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

    /* Load sounds — missing files are non-fatal */
    struct { SoundId id; const char *path; } sfx[] = {
        { SOUND_PICKUP,       "assets/sfx/pickup.wav" },
        { SOUND_DROP,         "assets/sfx/drop.wav" },
        { SOUND_DRAGON_BITE,  "assets/sfx/dragon_bite.wav" },
        { SOUND_DRAGON_DEATH, "assets/sfx/dragon_death.wav" },
        { SOUND_GATE_OPEN,    "assets/sfx/gate_open.wav" },
        { SOUND_GATE_CLOSE,   "assets/sfx/gate_close.wav" },
        { SOUND_BAT_SWAP,     "assets/sfx/bat_swap.wav" },
        { SOUND_WIN,          "assets/sfx/win.wav" },
        { SOUND_WALL_BUMP,    "assets/sfx/wall_bump.wav" },
        { SOUND_BRIDGE,       "assets/sfx/bridge.wav" },
    };

    for (int i = 0; i < (int)(sizeof(sfx) / sizeof(sfx[0])); i++)
    {
        _sounds[sfx[i].id] = MIX_LoadAudio(_mixer, sfx[i].path, true);
        if (!_sounds[sfx[i].id])
            SDL_Log("audio: failed to load %s (non-fatal)", sfx[i].path);
    }

    event_bus_subscribe(EVENT_PLAY_SOUND, _on_play_sound);
}

void audio_fini(void)
{
    for (int i = 0; i < SOUND_COUNT; i++)
    {
        if (_sounds[i])
        {
            MIX_DestroyAudio(_sounds[i]);
            _sounds[i] = NULL;
        }
    }

    if (_mixer)
    {
        MIX_DestroyMixer(_mixer);
        _mixer = NULL;
    }

    MIX_Quit();
}

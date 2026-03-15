#include "audio.h"

#include "../event_bus.h"

//==============================================================================

static void _on_play_sound(const void *data)
{
    const PlaySoundData *e = data;
    MIX_Audio *sound = sound_manager_get((SoundName)e->id);
    sound_manager_play(sound, e->volume);
}

//------------------------------------------------------------------------------

void audio_init(void)
{
    event_bus_subscribe(EVENT_PLAY_SOUND, _on_play_sound);
}

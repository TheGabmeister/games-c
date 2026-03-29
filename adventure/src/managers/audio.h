#ifndef AUDIO_MANAGER_H
#define AUDIO_MANAGER_H

typedef enum {
    SOUND_PICKUP,
    SOUND_DROP,
    SOUND_DRAGON_BITE,
    SOUND_DRAGON_DEATH,
    SOUND_GATE_OPEN,
    SOUND_GATE_CLOSE,
    SOUND_BAT_SWAP,
    SOUND_WIN,
    SOUND_WALL_BUMP,
    SOUND_BRIDGE,
    SOUND_COUNT
} SoundId;

void audio_init(void);
void audio_fini(void);

#endif

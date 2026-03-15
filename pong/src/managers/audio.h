#ifndef AUDIO_MANAGER_H
#define AUDIO_MANAGER_H

typedef enum {
    SOUND_BUMP,
    SOUND_COUNT
} SoundId;

void audio_init(void);
void audio_fini(void);

#endif

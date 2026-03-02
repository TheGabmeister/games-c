#ifndef AUDIO_H
#define AUDIO_H

typedef enum {
    SFX_LASER,
    SFX_BUMP,
    SFX_COUNT   /* sentinel — also means "no sound" */
} SfxId;

void audio_manager_init(void);
void audio_manager_destroy(void);
void audio_play_sfx(SfxId id);

#endif /* AUDIO_H */

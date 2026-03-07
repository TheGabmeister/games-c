#include "audio.h"

/* Audio stubs — wire up sound files here when assets are available */

void audio_init(void) {
    /* InitAudioDevice() is called from main.c before this */
}

void audio_shutdown(void) {
    /* CloseAudioDevice() is called from main.c */
}

void audio_update(const GameState *gs) {
    (void)gs; /* suppress unused warning */
}

#ifndef AUDIO_MANAGER_H
#define AUDIO_MANAGER_H

/* Initialise/teardown the audio device. */
void audio_manager_init(void);
void audio_manager_destroy(void);

/* Load an audio asset by string ID.
 * IDs prefixed with "music-" are loaded as streamed Music tracks;
 * all others are loaded as in-memory Sound effects. */
void audio_manager_add(const char *id, const char *file);

/* Play a previously loaded sound effect by ID. */
void audio_play_sfx(const char *id);

/* Start/stop the active music track. */
void audio_play_music(const char *id);
void audio_stop_music(void);

/* Must be called once per frame while music is playing. */
void audio_update(void);

#endif /* AUDIO_MANAGER_H */

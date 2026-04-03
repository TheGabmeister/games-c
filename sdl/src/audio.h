#ifndef AUDIO_H
#define AUDIO_H

#include <SDL3_mixer/SDL_mixer.h>

/*
 * Audio Helpers
 * =============
 * Simple wrappers around SDL_mixer for common game audio tasks.
 * Call audio_init() once at startup, audio_shutdown() at exit.
 *
 * --- Sound effects ---
 *
 * Short, fire-and-forget sounds (gunshots, UI clicks, pickups):
 *
 *     Mix_Chunk *jump_sfx = res_load_sound("assets/jump.wav");
 *     play_sound(jump_sfx);                  // default volume
 *     play_sound_ex(jump_sfx, 64, 0);        // half volume, no loop
 *
 * --- Music ---
 *
 * Streamed background music (only one track plays at a time):
 *
 *     Mix_Music *bgm = res_load_music("assets/bgm.mp3");
 *     play_music(bgm);             // loops forever
 *     play_music_ex(bgm, 0);       // play once
 *     pause_music();
 *     resume_music();
 *     stop_music();
 *
 * --- Volume ---
 *
 *     set_sound_volume(64);   // 0–128, affects all future sound effects
 *     set_music_volume(64);   // 0–128
 *
 * --- Supported formats ---
 *
 * This template is configured for WAV, MP3, and OGG Vorbis (all header-only
 * decoders, no external library dependencies).
 */

/* Initialize the audio subsystem. Call once after init_window(). */
void audio_init(void);

/* Shut down audio. Call before close_window(). */
void audio_shutdown(void);

/* Play a sound effect on the first available channel. */
void play_sound(Mix_Chunk *chunk);

/* Play a sound effect with volume (0–128) and loop count (0 = once, -1 = forever). */
void play_sound_ex(Mix_Chunk *chunk, int volume, int loops);

/* Play music, looping forever. Replaces any currently playing music. */
void play_music(Mix_Music *music);

/* Play music with a specific loop count (0 = once, -1 = forever). */
void play_music_ex(Mix_Music *music, int loops);

void pause_music(void);
void resume_music(void);
void stop_music(void);

/* Set master volume for sound effects and music (0–128). */
void set_sound_volume(int volume);
void set_music_volume(int volume);

#endif /* AUDIO_H */

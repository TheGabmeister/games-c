#ifndef AUDIO_H
#define AUDIO_H

#include <SDL3_mixer/SDL_mixer.h>

/*
 * Audio Helpers
 * =============
 * Simple wrappers around SDL3_mixer 3.x for common game audio tasks.
 * Call audio_init() once at startup, audio_shutdown() at exit.
 *
 * SDL3_mixer uses a mixer/track/audio model:
 *   - MIX_Mixer  — the mixer device (one per game, managed here)
 *   - MIX_Audio  — loaded audio data (load via res_load_sound/res_load_music)
 *   - MIX_Track  — a playback slot (created automatically by play_sound/play_music)
 *
 * --- Sound effects (fire-and-forget) ---
 *
 *     MIX_Audio *jump = res_load_sound("assets/jump.wav");
 *     play_sound(jump);    // plays once on an auto-created internal track
 *
 * --- Music (one track, supports looping/pause/resume) ---
 *
 *     MIX_Audio *bgm = res_load_music("assets/bgm.mp3");
 *     play_music(bgm);             // loops forever
 *     play_music_once(bgm);        // plays once
 *     pause_music();
 *     resume_music();
 *     stop_music();
 *
 * --- Volume ---
 *
 *     set_master_volume(0.5f);     // 0.0 = silent, 1.0 = full
 *     set_music_volume(0.8f);
 *
 * --- Advanced ---
 *
 *     get_mixer() returns the raw MIX_Mixer* for direct SDL3_mixer API calls.
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

/* Get the raw mixer for direct SDL3_mixer API access. */
MIX_Mixer *get_mixer(void);

/* Play a sound effect once (fire-and-forget). */
void play_sound(MIX_Audio *audio);

/* Play music, looping forever. Stops any previously playing music. */
void play_music(MIX_Audio *audio);

/* Play music once (no loop). Stops any previously playing music. */
void play_music_once(MIX_Audio *audio);

void pause_music(void);
void resume_music(void);
void stop_music(void);

/* Master volume: 0.0 = silent, 1.0 = full. Affects all output. */
void set_master_volume(float gain);

/* Music track volume: 0.0 = silent, 1.0 = full. */
void set_music_volume(float gain);

#endif /* AUDIO_H */

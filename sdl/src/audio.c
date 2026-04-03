#include "audio.h"
#include <SDL3/SDL.h>

void audio_init(void)
{
    if (!Mix_OpenAudio(0, NULL)) {
        SDL_Log("audio_init: Mix_OpenAudio failed: %s", SDL_GetError());
    }
}

void audio_shutdown(void)
{
    Mix_CloseAudio();
}

void play_sound(Mix_Chunk *chunk)
{
    if (!chunk) return;
    Mix_PlayChannel(-1, chunk, 0);
}

void play_sound_ex(Mix_Chunk *chunk, int volume, int loops)
{
    if (!chunk) return;
    int channel = Mix_PlayChannel(-1, chunk, loops);
    if (channel >= 0)
        Mix_Volume(channel, volume);
}

void play_music(Mix_Music *music)
{
    play_music_ex(music, -1);
}

void play_music_ex(Mix_Music *music, int loops)
{
    if (!music) return;
    Mix_PlayMusic(music, loops);
}

void pause_music(void)
{
    Mix_PauseMusic();
}

void resume_music(void)
{
    Mix_ResumeMusic();
}

void stop_music(void)
{
    Mix_HaltMusic();
}

void set_sound_volume(int volume)
{
    Mix_Volume(-1, volume);
}

void set_music_volume(int volume)
{
    Mix_VolumeMusic(volume);
}

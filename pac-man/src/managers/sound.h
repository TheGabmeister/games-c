#ifndef SOUND_MANAGER_H
#define SOUND_MANAGER_H

#include <flecs.h>
#include <SDL3_mixer/SDL_mixer.h>

typedef enum SoundName
{
    SOUND_KRANZKY,
    SOUND_SHOOT,
    SOUND_BOOM,
    MAX_SOUNDS
} SoundName;

void sound_manager_init(ecs_world_t *world);
MIX_Audio *sound_manager_get(SoundName id);
bool sound_manager_play(MIX_Audio *sound, float volume);

#endif

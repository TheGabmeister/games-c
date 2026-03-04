#ifndef MUSIC_MANAGER_H
#define MUSIC_MANAGER_H

#include <flecs.h>
#include <SDL3_mixer/SDL_mixer.h>

typedef enum MusicName
{
  MUSIC_ROCK_VOMIT,
  MAX_MUSIC
} MusicName;

void music_manager_init(ecs_world_t *world);
MIX_Audio *music_manager_get(MusicName id);
MIX_Mixer *music_manager_mixer(void);
void music_manager_volume(ecs_world_t *world, ecs_entity_t id, float volume);
void music_manager_mute(ecs_world_t *world, ecs_entity_t id);
void music_manager_unmute(ecs_world_t *world, ecs_entity_t id);
void music_manager_stop(ecs_world_t *world, ecs_entity_t id);

#endif

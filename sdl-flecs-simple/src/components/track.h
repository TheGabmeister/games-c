#ifndef TRACK_COMPONENT_H
#define TRACK_COMPONENT_H

#include <flecs.h>
#include <SDL3_mixer/SDL_mixer.h>

typedef enum TrackState
{
  TRACK_WAITING,
  TRACK_STARTING,
  TRACK_PLAYING,
  TRACK_MUTING,
  TRACK_MUTED,
  TRACK_UNMUTING,
  TRACK_STOPPING,
  TRACK_STOPPED,
  MAX_TRACKS
} TrackState;

typedef struct Track
{
  MIX_Audio *music;
  MIX_Track *mix_track;
  TrackState state;
  float volume;
  float length;
  float track_time;
  float state_time;
} Track;

ECS_COMPONENT_DECLARE(Track);

#endif

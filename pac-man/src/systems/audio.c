#include <SDL3/SDL.h>
#include <SDL3/SDL_properties.h>
#include <SDL3_mixer/SDL_mixer.h>

#include "../components/audible.h"
#include "../components/track.h"
#include "../managers/music.h"

//==============================================================================

static bool _audio_ready(void)
{
  return music_manager_mixer() != NULL;
}

//------------------------------------------------------------------------------

static float _clamp_gain(float volume)
{
  if (volume < 0.0f)
    return 0.0f;
  if (volume > 1.0f)
    return 1.0f;
  return volume;
}

//------------------------------------------------------------------------------

static void _destroy_music_track(Track *track)
{
  if (!track->mix_track)
    return;

  MIX_DestroyTrack(track->mix_track);
  track->mix_track = NULL;
}

//------------------------------------------------------------------------------

static bool _start_music_track(Track *track)
{
  SDL_PropertiesID props;

  if (!_audio_ready() || !track->music)
    return false;

  _destroy_music_track(track);

  track->mix_track = MIX_CreateTrack(music_manager_mixer());
  if (!track->mix_track)
    return false;

  if (!MIX_SetTrackAudio(track->mix_track, track->music))
  {
    _destroy_music_track(track);
    return false;
  }

  if (!MIX_SetTrackGain(track->mix_track, 0.0f))
  {
    _destroy_music_track(track);
    return false;
  }

  props = SDL_CreateProperties();
  if (!props)
  {
    _destroy_music_track(track);
    return false;
  }

  SDL_SetNumberProperty(props, MIX_PROP_PLAY_LOOPS_NUMBER, -1);
  if (!MIX_PlayTrack(track->mix_track, props))
  {
    SDL_DestroyProperties(props);
    _destroy_music_track(track);
    return false;
  }

  SDL_DestroyProperties(props);
  return true;
}

//------------------------------------------------------------------------------

void play_sounds(ecs_iter_t *it)
{
    Audible *audible = ecs_field(it, Audible, 0);
    for (int i = 0; i < it->count; ++i)
    {
        sound_manager_play(audible[i].sound, audible[i].volume);
        ecs_delete(it->world, it->entities[i]);
    }
}

//------------------------------------------------------------------------------

static inline void _update_track(Track *track, float volume, float delta)
{
  if (_audio_ready() && track->mix_track)
  {
    Sint64 frames = MIX_GetTrackPlaybackPosition(track->mix_track);
    Sint64 ms = MIX_TrackFramesToMS(track->mix_track, frames);

    MIX_SetTrackGain(track->mix_track, _clamp_gain(volume));
    if (ms >= 0)
      track->track_time = (float)ms / 1000.0f;
  }
  else
  {
    track->track_time += delta;
  }
  track->state_time += delta;
  if (track->state_time > 1)
    track->state_time = 1;
}

//------------------------------------------------------------------------------

void play_music(ecs_iter_t *it)
{
  Track *track = ecs_field(it, Track, 0);
  for (int i = 0; i < it->count; ++i)
  {
    TrackState new_state = MAX_TRACKS;
    switch (track[i].state)
    {
    case TRACK_WAITING:
      new_state = _start_music_track(&track[i]) ? TRACK_STARTING : TRACK_STOPPED;
      break;
    case TRACK_STARTING:
      _update_track(&track[i], track[i].volume * track[i].state_time, it->delta_time * 4);
      if (track[i].state_time >= 0.25)
        new_state = TRACK_PLAYING;
      break;
    case TRACK_PLAYING:
      _update_track(&track[i], track[i].volume, it->delta_time);
      break;
    case TRACK_MUTING:
      _update_track(&track[i], track[i].volume * (1 - track[i].state_time), it->delta_time * 4);
      if (track[i].state_time >= 0.25)
        new_state = TRACK_MUTED;
      break;
    case TRACK_MUTED:
      _update_track(&track[i], 0, it->delta_time);
      break;
    case TRACK_UNMUTING:
      _update_track(&track[i], track[i].volume * track[i].state_time, it->delta_time * 4);
      if (track[i].state_time >= 0.25)
        new_state = TRACK_PLAYING;
      break;
    case TRACK_STOPPING:
      _update_track(&track[i], track[i].volume * (1 - track[i].state_time), it->delta_time * 4);
      if (track[i].state_time >= 0.25)
        new_state = TRACK_STOPPED;
      break;
    case TRACK_STOPPED:
      if (track[i].mix_track)
        MIX_StopTrack(track[i].mix_track, 0);
      _destroy_music_track(&track[i]);
      ecs_delete(it->world, it->entities[i]);
      break;
    default:
      SDL_LogWarn(SDL_LOG_CATEGORY_AUDIO, "bad audio state");
      break;
    }
    if (new_state != MAX_TRACKS)
    {
      track[i].state = new_state;
      track[i].state_time = 0;
    }
  }
}

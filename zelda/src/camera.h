#ifndef CAMERA_H
#define CAMERA_H

#include "game_config.h"
#include "player.h"
#include <stdbool.h>

typedef enum TransitionType {
    TRANS_NONE = 0,
    TRANS_SCROLL,
    TRANS_FADE,
} TransitionType;

typedef enum FadePhase {
    FADE_NONE = 0,
    FADE_OUT,
    FADE_IN,
} FadePhase;

typedef struct TransitionCamera {
    TransitionType type;
    Direction dir;
    int timer;
    int total_frames;
    FadePhase fade_phase;
    bool active;
} TransitionCamera;

void camera_start_scroll(TransitionCamera *cam, Direction dir);
void camera_start_fade(TransitionCamera *cam);
void camera_update(TransitionCamera *cam);
bool camera_is_active(const TransitionCamera *cam);

void camera_scroll_offset_old(const TransitionCamera *cam, int *ox, int *oy);
void camera_scroll_offset_new(const TransitionCamera *cam, int *ox, int *oy);
unsigned char camera_fade_alpha(const TransitionCamera *cam);
bool camera_at_midpoint(const TransitionCamera *cam);

#endif

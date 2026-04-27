#include "camera.h"

void camera_start_scroll(TransitionCamera *cam, Direction dir) {
    cam->type = TRANS_SCROLL;
    cam->dir = dir;
    cam->timer = 0;
    cam->total_frames = SCROLL_FRAMES;
    cam->fade_phase = FADE_NONE;
    cam->active = true;
}

void camera_start_fade(TransitionCamera *cam) {
    cam->type = TRANS_FADE;
    cam->timer = 0;
    cam->total_frames = FADE_OUT_FRAMES + FADE_IN_FRAMES;
    cam->fade_phase = FADE_OUT;
    cam->active = true;
}

void camera_update(TransitionCamera *cam) {
    if (!cam->active) return;

    cam->timer++;

    if (cam->type == TRANS_FADE && cam->timer == FADE_OUT_FRAMES) {
        cam->fade_phase = FADE_IN;
    }

    if (cam->timer >= cam->total_frames) {
        cam->active = false;
        cam->type = TRANS_NONE;
        cam->fade_phase = FADE_NONE;
    }
}

bool camera_is_active(const TransitionCamera *cam) {
    return cam->active;
}

bool camera_at_midpoint(const TransitionCamera *cam) {
    return cam->type == TRANS_FADE && cam->timer == FADE_OUT_FRAMES;
}

void camera_scroll_offset_old(const TransitionCamera *cam, int *ox, int *oy) {
    *ox = 0;
    *oy = 0;
    if (!cam->active || cam->type != TRANS_SCROLL) return;

    float t = (float)cam->timer / cam->total_frames;
    int play_w = SCREEN_TILES_X * TILE_SIZE;
    int play_h = PLAY_AREA_HEIGHT;

    switch (cam->dir) {
        case DIR_N: *oy = (int)(t * play_h); break;
        case DIR_S: *oy = (int)(-t * play_h); break;
        case DIR_E: *ox = (int)(-t * play_w); break;
        case DIR_W: *ox = (int)(t * play_w); break;
        default: break;
    }
}

void camera_scroll_offset_new(const TransitionCamera *cam, int *ox, int *oy) {
    *ox = 0;
    *oy = 0;
    if (!cam->active || cam->type != TRANS_SCROLL) return;

    float t = (float)cam->timer / cam->total_frames;
    int play_w = SCREEN_TILES_X * TILE_SIZE;
    int play_h = PLAY_AREA_HEIGHT;

    switch (cam->dir) {
        case DIR_N: *oy = (int)(-play_h + t * play_h); break;
        case DIR_S: *oy = (int)(play_h - t * play_h); break;
        case DIR_E: *ox = (int)(play_w - t * play_w); break;
        case DIR_W: *ox = (int)(-play_w + t * play_w); break;
        default: break;
    }
}

unsigned char camera_fade_alpha(const TransitionCamera *cam) {
    if (!cam->active || cam->type != TRANS_FADE) return 0;

    if (cam->fade_phase == FADE_OUT) {
        return (unsigned char)(255 * cam->timer / FADE_OUT_FRAMES);
    } else {
        int fade_timer = cam->timer - FADE_OUT_FRAMES;
        return (unsigned char)(255 - 255 * fade_timer / FADE_IN_FRAMES);
    }
}

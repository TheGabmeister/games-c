#include "camera.h"

void camera_update(float *camera_x, Entity *mario, Level *level) {
    float mario_screen_x = mario->x - *camera_x;

    // Scroll right when Mario passes the threshold
    if (mario_screen_x > CAMERA_THRESHOLD) {
        *camera_x = mario->x - CAMERA_THRESHOLD;
    }

    // Never scroll left
    if (*camera_x < 0) *camera_x = 0;

    // Clamp at level end
    float max_cam = (float)(level->width * TILE_SIZE - WINDOW_WIDTH);
    if (max_cam < 0) max_cam = 0;
    if (*camera_x > max_cam) *camera_x = max_cam;
}

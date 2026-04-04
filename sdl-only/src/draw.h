#ifndef DRAW_H
#define DRAW_H

#include "platform.h"

// Color presets
#define COLOR_WHITE    (color){255, 255, 255, 255}
#define COLOR_BLACK    (color){0,   0,   0,   255}
#define COLOR_RED      (color){255, 0,   0,   255}
#define COLOR_GREEN    (color){0,   200, 0,   255}
#define COLOR_BLUE     (color){0,   0,   255, 255}
#define COLOR_YELLOW   (color){255, 255, 0,   255}
#define COLOR_CYAN     (color){0,   255, 255, 255}
#define COLOR_MAGENTA  (color){255, 0,   255, 255}
#define COLOR_ORANGE   (color){255, 160, 0,   255}
#define COLOR_GRAY     (color){128, 128, 128, 255}
#define COLOR_DARKGRAY (color){64,  64,  64,  255}

// Screen
void begin_drawing(color clear_color);
void end_drawing(void);

// Shapes
void draw_rect(rectangle rect, color c);
void draw_rect_filled(rectangle rect, color c);
void draw_line(vector2 start, vector2 end, color c);
void draw_circle(vector2 center, float radius, color c);
void draw_circle_filled(vector2 center, float radius, color c);

// Text (SDL built-in debug font, 8x8 pixels per character)
void draw_text(const char *text, float x, float y, float scale, color c);

#endif // DRAW_H

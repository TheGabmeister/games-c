#ifndef TYPES_H
#define TYPES_H

#include <math.h>
#include <stdbool.h>

typedef struct Vec2 {
    float x, y;
} Vec2;

typedef struct Color {
    unsigned char r, g, b, a;
} Color;

typedef struct Rect {
    float x, y, w, h;
} Rect;

/* Common colors */
#define COLOR_WHITE     ((Color){255, 255, 255, 255})
#define COLOR_BLACK     ((Color){0, 0, 0, 255})
#define COLOR_RED       ((Color){220, 50, 50, 255})
#define COLOR_GREEN     ((Color){50, 200, 50, 255})
#define COLOR_BLUE      ((Color){50, 100, 220, 255})
#define COLOR_YELLOW    ((Color){240, 220, 50, 255})
#define COLOR_GRAY      ((Color){128, 128, 128, 255})
#define COLOR_DARK_GRAY ((Color){40, 40, 40, 255})

/* Vec2 helpers */
static inline Vec2 vec2(float x, float y) { return (Vec2){x, y}; }
static inline Vec2 vec2_add(Vec2 a, Vec2 b) { return (Vec2){a.x + b.x, a.y + b.y}; }
static inline Vec2 vec2_sub(Vec2 a, Vec2 b) { return (Vec2){a.x - b.x, a.y - b.y}; }
static inline Vec2 vec2_scale(Vec2 v, float s) { return (Vec2){v.x * s, v.y * s}; }
static inline float vec2_dot(Vec2 a, Vec2 b) { return a.x * b.x + a.y * b.y; }
static inline float vec2_length_sq(Vec2 v) { return v.x * v.x + v.y * v.y; }
static inline float vec2_length(Vec2 v) { return sqrtf(vec2_length_sq(v)); }
static inline Vec2 vec2_normalize(Vec2 v) {
    float len = vec2_length(v);
    if (len < 0.0001f) return (Vec2){0, 0};
    return (Vec2){v.x / len, v.y / len};
}

/* Rect helpers */
static inline Rect rect_make(float x, float y, float w, float h) { return (Rect){x, y, w, h}; }
static inline Vec2 rect_center(Rect r) { return (Vec2){r.x + r.w * 0.5f, r.y + r.h * 0.5f}; }
static inline bool rect_contains(Rect r, Vec2 p) {
    return p.x >= r.x && p.x < r.x + r.w && p.y >= r.y && p.y < r.y + r.h;
}
static inline bool rect_overlaps(Rect a, Rect b) {
    return a.x < b.x + b.w && a.x + a.w > b.x &&
           a.y < b.y + b.h && a.y + a.h > b.y;
}

/* Clamp float to [min, max] */
static inline float clampf(float v, float min, float max) {
    if (v < min) return min;
    if (v > max) return max;
    return v;
}

/* Linear interpolation */
static inline float lerpf(float a, float b, float t) {
    return a + (b - a) * t;
}

#endif

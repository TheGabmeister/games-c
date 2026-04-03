#include "effects.h"

void InitStars(Star *stars)
{
    for (int i = 0; i < STAR_COUNT; i++) {
        stars[i].x = (float)GetRandomValue(0, SCREEN_W);
        stars[i].y = (float)GetRandomValue(0, SCREEN_H);
        int layer = GetRandomValue(0, 2);
        switch (layer) {
            case 0: stars[i].speed = 10; stars[i].bright = 0.3f; break;
            case 1: stars[i].speed = 20; stars[i].bright = 0.5f; break;
            default: stars[i].speed = 35; stars[i].bright = 0.8f; break;
        }
    }
}

void UpdateStars(Star *stars, float dt)
{
    for (int i = 0; i < STAR_COUNT; i++) {
        stars[i].y += stars[i].speed * dt;
        if (stars[i].y > SCREEN_H) {
            stars[i].y = 0;
            stars[i].x = (float)GetRandomValue(0, SCREEN_W);
        }
    }
}

void DrawStars(const Star *stars)
{
    for (int i = 0; i < STAR_COUNT; i++) {
        float sz = stars[i].bright * 1.5f;
        DrawCircleV((Vector2){ stars[i].x, stars[i].y }, sz, Fade(WHITE, stars[i].bright));
    }
}

void UpdateFloatTexts(FloatText *texts, float dt)
{
    for (int i = 0; i < FLOAT_TEXT_MAX; i++) {
        if (!texts[i].active) continue;
        texts[i].pos.y -= 40.0f * dt;
        texts[i].life -= dt;
        if (texts[i].life <= 0) texts[i].active = false;
    }
}

void DrawFloatTexts(const FloatText *texts)
{
    for (int i = 0; i < FLOAT_TEXT_MAX; i++) {
        if (!texts[i].active) continue;
        float alpha = texts[i].life;
        int w = MeasureText(texts[i].text, 16);
        DrawText(texts[i].text, (int)(texts[i].pos.x - w / 2), (int)texts[i].pos.y, 16,
                 Fade(texts[i].color, alpha));
    }
}

#include "drawing.h"
#include <math.h>

// --- Player Ship (chevron) ---
void DrawPlayerShip(float x, float y, bool blink)
{
    if (blink) return;

    Color col = COL_PLAYER;

    // Glow pass
    DrawTriangle(
        (Vector2){ x, y - 13 },
        (Vector2){ x + 18, y + 13 },
        (Vector2){ x - 18, y + 13 },
        Fade(col, 0.15f));

    // Solid body
    DrawTriangle(
        (Vector2){ x, y - 10 },
        (Vector2){ x + 15, y + 10 },
        (Vector2){ x - 15, y + 10 },
        col);

    // Cockpit
    DrawCircleV((Vector2){ x, y - 2 }, 3, (Color){ 0, 180, 200, 255 });

    // Engine glow
    DrawRectangle((int)(x - 4), (int)(y + 8), 8, 4, Fade(WHITE, 0.5f));
}

// --- Alien Shapes ---
void DrawAlienShape(AlienType type, float x, float y, int frame)
{
    Color col = AlienColor(type);
    Color glow = Fade(col, 0.15f);

    switch (type) {
    case AT_TOP: {
        // Glow diamond
        DrawTriangle((Vector2){ x, y - 10 }, (Vector2){ x + 12, y }, (Vector2){ x - 12, y }, glow);
        DrawTriangle((Vector2){ x, y + 10 }, (Vector2){ x - 12, y }, (Vector2){ x + 12, y }, glow);
        // Solid diamond
        DrawTriangle((Vector2){ x, y - 8 }, (Vector2){ x + 10, y }, (Vector2){ x - 10, y }, col);
        DrawTriangle((Vector2){ x, y + 8 }, (Vector2){ x - 10, y }, (Vector2){ x + 10, y }, col);
        if (frame == 0) {
            DrawCircleV((Vector2){ x - 7, y + 5 }, 2, col);
            DrawCircleV((Vector2){ x + 7, y + 5 }, 2, col);
        } else {
            DrawLineEx((Vector2){ x - 10, y }, (Vector2){ x - 14, y + 6 }, 2, col);
            DrawLineEx((Vector2){ x + 10, y }, (Vector2){ x + 14, y + 6 }, 2, col);
        }
    } break;

    case AT_MID: {
        // Glow
        DrawCircleV((Vector2){ x, y }, 9, glow);
        // Solid body
        DrawCircleV((Vector2){ x, y }, 7, col);
        if (frame == 0) {
            DrawLineEx((Vector2){ x - 4, y - 7 }, (Vector2){ x - 6, y - 12 }, 2, col);
            DrawLineEx((Vector2){ x + 4, y - 7 }, (Vector2){ x + 6, y - 12 }, 2, col);
            DrawCircleV((Vector2){ x - 6, y - 12 }, 2, col);
            DrawCircleV((Vector2){ x + 6, y - 12 }, 2, col);
        } else {
            DrawLineEx((Vector2){ x - 4, y - 7 }, (Vector2){ x - 10, y - 10 }, 2, col);
            DrawLineEx((Vector2){ x + 4, y - 7 }, (Vector2){ x + 10, y - 10 }, 2, col);
            DrawCircleV((Vector2){ x - 10, y - 10 }, 2, col);
            DrawCircleV((Vector2){ x + 10, y - 10 }, 2, col);
        }
    } break;

    case AT_BOT: {
        // Glow
        DrawRectangle((int)(x - 8), (int)(y - 8), 16, 16, glow);
        // Solid body
        DrawRectangle((int)(x - 6), (int)(y - 6), 12, 12, col);
        if (frame == 0) {
            // Wings up
            DrawTriangle((Vector2){ x - 6, y - 6 }, (Vector2){ x - 6, y + 2 }, (Vector2){ x - 14, y - 6 }, col);
            DrawTriangle((Vector2){ x + 6, y - 6 }, (Vector2){ x + 14, y - 6 }, (Vector2){ x + 6, y + 2 }, col);
        } else {
            // Wings down
            DrawTriangle((Vector2){ x - 6, y - 2 }, (Vector2){ x - 6, y + 6 }, (Vector2){ x - 14, y + 6 }, col);
            DrawTriangle((Vector2){ x + 6, y - 2 }, (Vector2){ x + 14, y + 6 }, (Vector2){ x + 6, y + 6 }, col);
        }
    } break;
    }
}

// --- Shield ---
void DrawShieldBlock(Shield *s, Vector2 offset)
{
    for (int r = 0; r < SHIELD_ROWS; r++) {
        for (int c = 0; c < SHIELD_COLS; c++) {
            if (s->cells[r][c] == CELL_DEAD) continue;
            float cx = s->x + c * SHIELD_CELL + offset.x;
            float cy = s->y + r * SHIELD_CELL + offset.y;
            Color col = (s->cells[r][c] == CELL_FLASH) ? WHITE : COL_SHIELD;
            DrawRectangle((int)cx, (int)cy, SHIELD_CELL, SHIELD_CELL, col);
        }
    }
}

// --- Bullet ---
void DrawBulletObj(Bullet *b, bool isPlayer)
{
    if (!b->active) return;

    // Trail
    float alphas[] = { 0.50f, 0.25f, 0.10f };
    for (int i = 0; i < b->trailN; i++) {
        float sz = (float)BULLET_W * (1.0f - (float)i * 0.2f);
        DrawRectangle(
            (int)(b->trail[i].x - sz / 2),
            (int)(b->trail[i].y - BULLET_H / 2),
            (int)sz, BULLET_H,
            Fade(b->color, alphas[i]));
    }

    // Glow
    if (isPlayer) {
        DrawRectangle((int)(b->pos.x - BULLET_W / 2 - 1), (int)(b->pos.y - BULLET_H / 2 - 1),
                       BULLET_W + 2, BULLET_H + 2, Fade(COL_PLAYER, 0.3f));
        DrawRectangle((int)(b->pos.x - BULLET_W / 2), (int)(b->pos.y - BULLET_H / 2),
                       BULLET_W, BULLET_H, WHITE);
    } else {
        DrawRectangle((int)(b->pos.x - BULLET_W / 2 - 1), (int)(b->pos.y - BULLET_H / 2 - 1),
                       BULLET_W + 2, BULLET_H + 2, Fade(COL_ENEMY_FIRE, 0.3f));
        DrawRectangle((int)(b->pos.x - BULLET_W / 2), (int)(b->pos.y - BULLET_H / 2),
                       BULLET_W, BULLET_H, COL_ENEMY_FIRE);
    }
}

// --- UFO ---
void DrawUFOShape(UFOShip *u)
{
    if (!u->active) return;

    float cx = u->x, cy = u->y;
    Color col = (Color){ 255, 50, 50, 255 };

    // Pulsing glow
    float pulse = 0.10f + 0.05f * sinf(u->glowT * 6.0f);
    DrawCircleV((Vector2){ cx, cy }, 24, Fade(col, pulse));

    // Body
    DrawRectangle((int)(cx - 20), (int)(cy - 4), 40, 8, col);
    DrawCircleV((Vector2){ cx - 20, cy }, 4, col);
    DrawCircleV((Vector2){ cx + 20, cy }, 4, col);

    // Dome
    DrawCircleV((Vector2){ cx, cy - 6 }, 8, Fade(col, 0.8f));

    // Lights
    DrawCircleV((Vector2){ cx - 12, cy }, 2, WHITE);
    DrawCircleV((Vector2){ cx, cy }, 2, YELLOW);
    DrawCircleV((Vector2){ cx + 12, cy }, 2, WHITE);
}

// --- HUD ---
void DrawHUD(int score, int wave, int lives)
{
    // Score (left)
    DrawText(TextFormat("SCORE: %d", score), PLAY_LEFT, HUD_TOP, 20, WHITE);

    // Wave (center)
    const char *waveText = TextFormat("WAVE %d", wave);
    int waveW = MeasureText(waveText, 20);
    DrawText(waveText, SCREEN_W / 2 - waveW / 2, HUD_TOP, 20, COL_UI_CYAN);

    // Lives (right) — small ship icons
    for (int i = 0; i < lives; i++) {
        float lx = (float)(PLAY_RIGHT - 30 - i * 25);
        float ly = (float)(HUD_TOP + 10);
        DrawTriangle(
            (Vector2){ lx, ly - 6 },
            (Vector2){ lx + 8, ly + 6 },
            (Vector2){ lx - 8, ly + 6 },
            COL_PLAYER);
    }
}

// --- Ground Line ---
void DrawGroundLine(Vector2 offset)
{
    DrawLineEx(
        (Vector2){ PLAY_LEFT + offset.x, GROUND_Y + offset.y },
        (Vector2){ PLAY_RIGHT + offset.x, GROUND_Y + offset.y },
        2, Fade(COL_UI_CYAN, 0.3f));
}

// --- Starfield ---
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

void DrawStars(Star *stars)
{
    for (int i = 0; i < STAR_COUNT; i++) {
        float sz = stars[i].bright * 1.5f;
        DrawCircleV((Vector2){ stars[i].x, stars[i].y }, sz, Fade(WHITE, stars[i].bright));
    }
}

// --- Floating Score Texts ---
void UpdateFloatTexts(FloatText *texts, float dt)
{
    for (int i = 0; i < FLOAT_TEXT_MAX; i++) {
        if (!texts[i].active) continue;
        texts[i].pos.y -= 40.0f * dt;
        texts[i].life -= dt;
        if (texts[i].life <= 0) texts[i].active = false;
    }
}

void DrawFloatTexts(FloatText *texts)
{
    for (int i = 0; i < FLOAT_TEXT_MAX; i++) {
        if (!texts[i].active) continue;
        float alpha = texts[i].life;
        int w = MeasureText(texts[i].text, 16);
        DrawText(texts[i].text, (int)(texts[i].pos.x - w / 2), (int)texts[i].pos.y, 16,
                 Fade(texts[i].color, alpha));
    }
}

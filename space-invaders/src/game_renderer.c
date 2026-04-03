#include "game_renderer.h"
#include "drawing.h"
#include "effects.h"
#include "particles.h"

static Vector2 GetShakeOffset(const GameState *g)
{
    if (g->shakeT <= 0 || g->shakeDuration <= 0) return (Vector2){ 0, 0 };

    float t = g->shakeT / g->shakeDuration;
    if (t > 1.0f) t = 1.0f;

    float ox = (float)GetRandomValue(-100, 100) / 100.0f * g->shakeAmp * t;
    float oy = (float)GetRandomValue(-100, 100) / 100.0f * g->shakeAmp * t;
    return (Vector2){ ox, oy };
}

void GameDraw(const GameState *g)
{
    Vector2 shake = GetShakeOffset(g);

    DrawStars(g->stars);

    for (int s = 0; s < SHIELD_COUNT; s++)
        DrawShieldBlock(&g->shields[s], shake);

    for (int r = 0; r < ALIEN_ROWS; r++) {
        for (int c = 0; c < ALIEN_COLS; c++) {
            if (!g->aliens[r][c].alive) continue;
            float ax = g->formX + c * ALIEN_SPACING_X + shake.x;
            float ay = g->formY + r * ALIEN_SPACING_Y + shake.y;
            DrawAlienShape(g->aliens[r][c].type, ax, ay, g->animFrame);
        }
    }

    if (g->ufo.active) {
        UFOShip tempUfo = g->ufo;
        tempUfo.x += shake.x;
        tempUfo.y += shake.y;
        DrawUFOShape(&tempUfo);
    }

    if (g->pBullet.active) {
        Bullet bullet = g->pBullet;
        bullet.pos.x += shake.x;
        bullet.pos.y += shake.y;
        for (int t = 0; t < bullet.trailN; t++) {
            bullet.trail[t].x += shake.x;
            bullet.trail[t].y += shake.y;
        }
        DrawBulletObj(&bullet, true);
    }

    for (int i = 0; i < MAX_ENEMY_BULLETS; i++) {
        if (!g->eBullets[i].active) continue;
        Bullet bullet = g->eBullets[i];
        bullet.pos.x += shake.x;
        bullet.pos.y += shake.y;
        for (int t = 0; t < bullet.trailN; t++) {
            bullet.trail[t].x += shake.x;
            bullet.trail[t].y += shake.y;
        }
        DrawBulletObj(&bullet, false);
    }

    if (g->playerActive) {
        bool blink = false;
        if (g->invulnT > 0)
            blink = ((int)(g->invulnT * 10) % 2) == 0;
        DrawPlayerShip(g->playerX + shake.x, PLAYER_BASE_Y + shake.y, blink);
    }

    DrawParticles(g->particles);
    DrawFloatTexts(g->floatTexts);
    DrawGroundLine(shake);
    DrawHUD(g->score, g->wave, g->lives);

    if (g->waveComplete) {
        const char *text = "WAVE COMPLETE";
        int tw = MeasureText(text, 40);
        DrawText(text, SCREEN_W / 2 - tw / 2 + 2, SCREEN_H / 2 - 20 + 2, 40, Fade(COL_UI_CYAN, 0.3f));
        DrawText(text, SCREEN_W / 2 - tw / 2, SCREEN_H / 2 - 20, 40, WHITE);
    }

    if (g->dead) {
        const char *text = "GAME OVER";
        int tw = MeasureText(text, 40);
        DrawText(text, SCREEN_W / 2 - tw / 2 + 2, SCREEN_H / 2 - 20 + 2, 40, Fade((Color){ 255, 50, 50, 255 }, 0.3f));
        DrawText(text, SCREEN_W / 2 - tw / 2, SCREEN_H / 2 - 20, 40, (Color){ 255, 50, 50, 255 });
    }
}

#include "ui.h"

#include <math.h>

void UIDrawHUD(const GameContext *ctx)
{
    DrawText(TextFormat("%d", ctx->game.score), 10, 10, 20, WHITE);

    const char *hiText = TextFormat("HI %d", ctx->game.highScore);
    int hiWidth = MeasureText(hiText, 20);
    DrawText(hiText, SCREEN_WIDTH / 2 - hiWidth / 2, 10, 20, GRAY);

    DrawText(TextFormat("WAVE %d", ctx->game.wave), SCREEN_WIDTH - 110, 10, 20, GRAY);

    for (int i = 0; i < ctx->game.lives; i++) {
        float cx = 20.0f + (float)i * 22.0f;
        float cy = 42.0f;
        float s = 7.0f;
        Vector2 lv0 = { cx, cy - s };
        Vector2 lv1 = { cx - s * 0.6f, cy + s * 0.7f };
        Vector2 lv2 = { cx + s * 0.6f, cy + s * 0.7f };
        DrawTriangleLines(lv0, lv2, lv1, GREEN);
    }
}

void UIDrawTitleScreen(const GameContext *ctx)
{
    const char *title = "ASTEROIDS";
    int titleWidth = MeasureText(title, 60);
    DrawText(title, SCREEN_WIDTH / 2 - titleWidth / 2 + 1, SCREEN_HEIGHT / 2 - 80 + 1, 60,
             ColorAlpha(GREEN, 0.3f));
    DrawText(title, SCREEN_WIDTH / 2 - titleWidth / 2, SCREEN_HEIGHT / 2 - 80, 60, GREEN);

    if (sinf((float)GetTime() * 5.0f) > 0) {
        const char *start = "PRESS FIRE TO START";
        int startWidth = MeasureText(start, 20);
        DrawText(start, SCREEN_WIDTH / 2 - startWidth / 2, SCREEN_HEIGHT / 2, 20, WHITE);
    }

    const char *ctrl = "WASD / ARROWS / DPAD to move   SPACE / A to fire";
    int ctrlWidth = MeasureText(ctrl, 14);
    DrawText(ctrl, SCREEN_WIDTH / 2 - ctrlWidth / 2, SCREEN_HEIGHT / 2 + 50, 14, GRAY);

    if (ctx->game.highScore > 0) {
        const char *hi = TextFormat("HIGH SCORE: %d", ctx->game.highScore);
        int hiWidth = MeasureText(hi, 18);
        DrawText(hi, SCREEN_WIDTH / 2 - hiWidth / 2, SCREEN_HEIGHT / 2 + 80, 18, YELLOW);
    }
}

void UIDrawGameOverScreen(const GameContext *ctx)
{
    const char *go = "GAME OVER";
    int goWidth = MeasureText(go, 50);
    DrawText(go, SCREEN_WIDTH / 2 - goWidth / 2 + 1, SCREEN_HEIGHT / 2 - 60 + 1, 50,
             ColorAlpha(RED, 0.3f));
    DrawText(go, SCREEN_WIDTH / 2 - goWidth / 2, SCREEN_HEIGHT / 2 - 60, 50, RED);

    const char *score = TextFormat("SCORE: %d", ctx->game.score);
    int scoreWidth = MeasureText(score, 24);
    DrawText(score, SCREEN_WIDTH / 2 - scoreWidth / 2, SCREEN_HEIGHT / 2, 24, WHITE);

    if (ctx->game.gameOverTimer <= 0 && sinf((float)GetTime() * 5.0f) > 0) {
        const char *cont = "PRESS FIRE TO CONTINUE";
        int contWidth = MeasureText(cont, 18);
        DrawText(cont, SCREEN_WIDTH / 2 - contWidth / 2, SCREEN_HEIGHT / 2 + 40, 18, GRAY);
    }
}

#include "background.h"

void BackgroundInitStars(GameContext *ctx)
{
    for (int i = 0; i < MAX_STARS; i++) {
        ctx->stars[i].position = (Vector2){
            (float)GetRandomValue(0, SCREEN_WIDTH),
            (float)GetRandomValue(0, SCREEN_HEIGHT)
        };
        ctx->stars[i].layer = GetRandomValue(0, STAR_LAYERS - 1);
        ctx->stars[i].brightness = 0.2f + 0.3f * (float)ctx->stars[i].layer;
        ctx->stars[i].size = 1.0f + (float)ctx->stars[i].layer * 0.5f;
    }
}

void BackgroundDrawStarfield(const GameContext *ctx)
{
    for (int i = 0; i < MAX_STARS; i++) {
        float ox = -ctx->ship.velocity.x * 0.001f * (float)(ctx->stars[i].layer + 1);
        float oy = -ctx->ship.velocity.y * 0.001f * (float)(ctx->stars[i].layer + 1);
        float sx = ctx->stars[i].position.x + ox;
        float sy = ctx->stars[i].position.y + oy;

        if (sx < 0) sx += SCREEN_WIDTH;
        if (sx > SCREEN_WIDTH) sx -= SCREEN_WIDTH;
        if (sy < 0) sy += SCREEN_HEIGHT;
        if (sy > SCREEN_HEIGHT) sy -= SCREEN_HEIGHT;

        DrawCircleV((Vector2){ sx, sy }, ctx->stars[i].size,
                    ColorAlpha(WHITE, ctx->stars[i].brightness));
    }
}

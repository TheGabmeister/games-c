#include "background.h"

#include <math.h>

static Vector2 BackgroundParallaxVelocity(const GameContext *ctx)
{
    if (ctx->game.state == STATE_PLAYING || ctx->game.state == STATE_GAMEOVER) {
        return ctx->ship.velocity;
    }

    float t = (float)GetTime();
    return (Vector2){
        sinf(t * 0.2f) * 40.0f,
        cosf(t * 0.15f) * 25.0f
    };
}

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
    Vector2 parallaxVelocity = BackgroundParallaxVelocity(ctx);

    for (int i = 0; i < MAX_STARS; i++) {
        float ox = -parallaxVelocity.x * 0.001f * (float)(ctx->stars[i].layer + 1);
        float oy = -parallaxVelocity.y * 0.001f * (float)(ctx->stars[i].layer + 1);
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

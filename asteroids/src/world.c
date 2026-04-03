#include "world.h"

Vector2 WorldWrapPosition(Vector2 pos)
{
    float margin = ASTEROID_LARGE_R;
    if (pos.x < -margin) pos.x += SCREEN_WIDTH + 2 * margin;
    if (pos.x > SCREEN_WIDTH + margin) pos.x -= SCREEN_WIDTH + 2 * margin;
    if (pos.y < -margin) pos.y += SCREEN_HEIGHT + 2 * margin;
    if (pos.y > SCREEN_HEIGHT + margin) pos.y -= SCREEN_HEIGHT + 2 * margin;
    return pos;
}

void WorldUpdateScreenShake(GameContext *ctx, float dt)
{
    if (ctx->game.screenShakeTimer > 0) ctx->game.screenShakeTimer -= dt;
    if (ctx->game.screenShakeTimer < 0) ctx->game.screenShakeTimer = 0;
}

Camera2D WorldBuildCamera(const GameContext *ctx)
{
    Camera2D camera = { 0 };
    camera.target = (Vector2){ SCREEN_WIDTH / 2.0f, SCREEN_HEIGHT / 2.0f };
    camera.offset = (Vector2){ SCREEN_WIDTH / 2.0f, SCREEN_HEIGHT / 2.0f };
    camera.zoom = 1.0f;

    if (ctx->game.screenShakeTimer > 0) {
        float intensity = ctx->game.screenShakeMagnitude *
            (ctx->game.screenShakeTimer / SCREEN_SHAKE_DURATION);
        camera.target.x += (float)GetRandomValue(-10, 10) / 10.0f * intensity;
        camera.target.y += (float)GetRandomValue(-10, 10) / 10.0f * intensity;
    }

    return camera;
}

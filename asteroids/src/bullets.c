#include "bullets.h"

#include "render_fx.h"
#include "ship.h"
#include "world.h"

#include "raymath.h"

#include <math.h>

void BulletsFire(GameContext *ctx)
{
    for (int i = 0; i < MAX_BULLETS; i++) {
        if (ctx->bullets[i].active) continue;

        Vector2 nose = ShipGetVertex(ctx, 0);
        float rad = ctx->ship.rotation * DEG2RAD;
        Vector2 dir = { sinf(rad), -cosf(rad) };

        ctx->bullets[i].position = nose;
        ctx->bullets[i].velocity = (Vector2){
            dir.x * BULLET_SPEED + ctx->ship.velocity.x,
            dir.y * BULLET_SPEED + ctx->ship.velocity.y
        };
        ctx->bullets[i].lifetime = BULLET_LIFETIME;
        ctx->bullets[i].active = true;
        break;
    }
}

void BulletsUpdate(GameContext *ctx, float dt)
{
    for (int i = 0; i < MAX_BULLETS; i++) {
        if (!ctx->bullets[i].active) continue;

        ctx->bullets[i].position.x += ctx->bullets[i].velocity.x * dt;
        ctx->bullets[i].position.y += ctx->bullets[i].velocity.y * dt;
        ctx->bullets[i].position = WorldWrapPosition(ctx->bullets[i].position);
        ctx->bullets[i].lifetime -= dt;

        if (ctx->bullets[i].lifetime <= 0) ctx->bullets[i].active = false;
    }
}

void BulletsDraw(const GameContext *ctx)
{
    for (int i = 0; i < MAX_BULLETS; i++) {
        if (!ctx->bullets[i].active) continue;

        DrawNeonCircle(ctx->bullets[i].position, BULLET_RADIUS, WHITE);

        Vector2 vel = ctx->bullets[i].velocity;
        float len = Vector2Length(vel);
        if (len > 0) {
            Vector2 trail = {
                ctx->bullets[i].position.x - (vel.x / len) * 8.0f,
                ctx->bullets[i].position.y - (vel.y / len) * 8.0f
            };
            DrawNeonLine(ctx->bullets[i].position, trail, ColorAlpha(WHITE, 0.5f));
        }
    }
}

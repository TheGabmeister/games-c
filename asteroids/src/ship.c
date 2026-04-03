#include "ship.h"

#include "bullets.h"
#include "input.h"
#include "particles.h"
#include "render_fx.h"
#include "world.h"

#include "raymath.h"

#include <math.h>

static float DampingFactor(float perFrameDamping, float dt)
{
    return powf(perFrameDamping, dt * 60.0f);
}

static Vector2 RotateOffset(Vector2 offset, float angleDeg)
{
    float rad = angleDeg * DEG2RAD;
    float c = cosf(rad);
    float s = sinf(rad);
    return (Vector2){ offset.x * c - offset.y * s, offset.x * s + offset.y * c };
}

static void GetThrusterGeometry(const GameContext *ctx,
                                Vector2 *baseLeft,
                                Vector2 *baseRight,
                                Vector2 *mid)
{
    Vector2 left = ShipGetVertex(ctx, 1);
    Vector2 right = ShipGetVertex(ctx, 2);

    *baseLeft = (Vector2){
        left.x * 0.7f + right.x * 0.3f,
        left.y * 0.7f + right.y * 0.3f
    };
    *baseRight = (Vector2){
        left.x * 0.3f + right.x * 0.7f,
        left.y * 0.3f + right.y * 0.7f
    };
    *mid = (Vector2){
        (baseLeft->x + baseRight->x) * 0.5f,
        (baseLeft->y + baseRight->y) * 0.5f
    };
}

Vector2 ShipGetVertex(const GameContext *ctx, int index)
{
    Vector2 offsets[3] = {
        { 0.0f, -SHIP_SIZE },
        { -SHIP_SIZE * 0.6f, SHIP_SIZE * 0.7f },
        { SHIP_SIZE * 0.6f, SHIP_SIZE * 0.7f },
    };
    Vector2 rotated = RotateOffset(offsets[index], ctx->ship.rotation);
    return (Vector2){
        ctx->ship.position.x + rotated.x,
        ctx->ship.position.y + rotated.y
    };
}

void ShipInit(GameContext *ctx)
{
    ctx->ship.position = (Vector2){ SCREEN_WIDTH / 2.0f, SCREEN_HEIGHT / 2.0f };
    ctx->ship.velocity = (Vector2){ 0 };
    ctx->ship.rotation = 0;
    ctx->ship.alive = true;
    ctx->ship.thrusting = false;
    ctx->ship.invulnTimer = SHIP_INVULN_TIME;
    ctx->ship.fireCooldown = 0;
    ctx->ship.respawnTimer = 0;
}

void ShipUpdate(GameContext *ctx, float dt)
{
    if (!ctx->ship.alive) {
        ctx->ship.respawnTimer -= dt;
        if (ctx->ship.respawnTimer <= 0) {
            if (ctx->game.lives > 0) {
                ShipInit(ctx);
            } else {
                ctx->game.state = STATE_GAMEOVER;
                ctx->game.gameOverTimer = 2.0f;
                if (ctx->game.score > ctx->game.highScore) {
                    ctx->game.highScore = ctx->game.score;
                }
            }
        }
        return;
    }

    if (InputActionDown(ctx, ACTION_LEFT)) ctx->ship.rotation -= SHIP_ROTATION_SPEED * dt;
    if (InputActionDown(ctx, ACTION_RIGHT)) ctx->ship.rotation += SHIP_ROTATION_SPEED * dt;

    ctx->ship.thrusting = InputActionDown(ctx, ACTION_UP);
    if (ctx->ship.thrusting) {
        float rad = ctx->ship.rotation * DEG2RAD;
        Vector2 dir = { sinf(rad), -cosf(rad) };
        ctx->ship.velocity.x += dir.x * SHIP_THRUST_ACCEL * dt;
        ctx->ship.velocity.y += dir.y * SHIP_THRUST_ACCEL * dt;

        float speed = Vector2Length(ctx->ship.velocity);
        if (speed > SHIP_MAX_SPEED) {
            ctx->ship.velocity = Vector2Scale(ctx->ship.velocity, SHIP_MAX_SPEED / speed);
        }

        if (GetRandomValue(0, 1)) {
            Vector2 baseLeft;
            Vector2 baseRight;
            Vector2 mid;
            GetThrusterGeometry(ctx, &baseLeft, &baseRight, &mid);
            ParticlesSpawn(ctx, mid, 1, 50.0f, 1.0f, 2.0f, ORANGE);
        }
    }

    ctx->ship.velocity = Vector2Scale(ctx->ship.velocity, DampingFactor(SHIP_DRAG, dt));

    ctx->ship.position.x += ctx->ship.velocity.x * dt;
    ctx->ship.position.y += ctx->ship.velocity.y * dt;
    ctx->ship.position = WorldWrapPosition(ctx->ship.position);

    if (ctx->ship.invulnTimer > 0) ctx->ship.invulnTimer -= dt;

    ctx->ship.fireCooldown -= dt;
    if (InputActionPressed(ctx, ACTION_FIRE) && ctx->ship.fireCooldown <= 0) {
        BulletsFire(ctx);
        ctx->ship.fireCooldown = FIRE_COOLDOWN;
    }
}

void ShipDraw(const GameContext *ctx)
{
    if (!ctx->ship.alive) return;

    if (ctx->ship.invulnTimer > 0 &&
        fmodf(ctx->ship.invulnTimer, SHIP_BLINK_RATE * 2.0f) > SHIP_BLINK_RATE) {
        return;
    }

    Vector2 v0 = ShipGetVertex(ctx, 0);
    Vector2 v1 = ShipGetVertex(ctx, 1);
    Vector2 v2 = ShipGetVertex(ctx, 2);
    DrawNeonTriangleLines(v0, v1, v2, GREEN);
}

void ShipDrawThrustFlame(const GameContext *ctx)
{
    if (!ctx->ship.alive || !ctx->ship.thrusting) return;
    if (ctx->ship.invulnTimer > 0 &&
        fmodf(ctx->ship.invulnTimer, SHIP_BLINK_RATE * 2.0f) > SHIP_BLINK_RATE) {
        return;
    }

    Vector2 baseLeft;
    Vector2 baseRight;
    Vector2 mid;
    GetThrusterGeometry(ctx, &baseLeft, &baseRight, &mid);

    float flameLen = SHIP_SIZE * (0.5f + 0.5f * (float)GetRandomValue(50, 100) / 100.0f);
    float rad = ctx->ship.rotation * DEG2RAD;
    Vector2 tailDir = { -sinf(rad), cosf(rad) };
    Vector2 tip = { mid.x + tailDir.x * flameLen, mid.y + tailDir.y * flameLen };

    Color flameColor = GetRandomValue(0, 1) ? ORANGE : YELLOW;
    DrawNeonLine(baseLeft, tip, flameColor);
    DrawNeonLine(baseRight, tip, flameColor);
}

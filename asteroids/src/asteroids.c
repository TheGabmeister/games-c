#include "asteroids.h"

#include "particles.h"
#include "render_fx.h"
#include "world.h"

#include "raymath.h"

#include <math.h>

static void GenerateAsteroidShape(Asteroid *asteroid)
{
    asteroid->vertCount = GetRandomValue(ASTEROID_VERT_MIN, ASTEROID_VERT_MAX);
    for (int i = 0; i < asteroid->vertCount; i++) {
        float jag = (float)GetRandomValue(-100, 100) / 100.0f * ASTEROID_JAGGEDNESS;
        asteroid->vertDistances[i] = asteroid->radius * (1.0f + jag);
    }
}

static float AsteroidRadius(AsteroidSize size)
{
    switch (size) {
        case ASTEROID_LARGE: return ASTEROID_LARGE_R;
        case ASTEROID_MEDIUM: return ASTEROID_MEDIUM_R;
        case ASTEROID_SMALL: return ASTEROID_SMALL_R;
    }
    return ASTEROID_LARGE_R;
}

static void SpawnAsteroid(GameContext *ctx, AsteroidSize size, Vector2 pos, Vector2 vel)
{
    for (int i = 0; i < MAX_ASTEROIDS; i++) {
        if (ctx->asteroids[i].active) continue;

        ctx->asteroids[i].position = pos;
        ctx->asteroids[i].velocity = vel;
        ctx->asteroids[i].size = size;
        ctx->asteroids[i].radius = AsteroidRadius(size);
        ctx->asteroids[i].rotation = (float)GetRandomValue(0, 360);
        ctx->asteroids[i].rotationSpeed = (float)GetRandomValue(-90, 90);
        ctx->asteroids[i].active = true;
        GenerateAsteroidShape(&ctx->asteroids[i]);
        break;
    }
}

void AsteroidsSpawnWave(GameContext *ctx, int count)
{
    for (int n = 0; n < count; n++) {
        Vector2 pos;
        int edge = GetRandomValue(0, 3);
        switch (edge) {
            case 0:
                pos = (Vector2){ (float)GetRandomValue(0, SCREEN_WIDTH), -ASTEROID_LARGE_R };
                break;
            case 1:
                pos = (Vector2){ (float)GetRandomValue(0, SCREEN_WIDTH), SCREEN_HEIGHT + ASTEROID_LARGE_R };
                break;
            case 2:
                pos = (Vector2){ -ASTEROID_LARGE_R, (float)GetRandomValue(0, SCREEN_HEIGHT) };
                break;
            default:
                pos = (Vector2){ SCREEN_WIDTH + ASTEROID_LARGE_R, (float)GetRandomValue(0, SCREEN_HEIGHT) };
                break;
        }

        Vector2 center = { SCREEN_WIDTH / 2.0f, SCREEN_HEIGHT / 2.0f };
        Vector2 dir = Vector2Subtract(center, pos);
        float angle = atan2f(dir.y, dir.x) + (float)GetRandomValue(-30, 30) * DEG2RAD;
        float speed = ASTEROID_SPEED_MIN +
            (ASTEROID_SPEED_MAX - ASTEROID_SPEED_MIN) * (float)GetRandomValue(0, 100) / 100.0f;
        speed *= 1.0f + (ctx->game.wave - 1) * 0.1f;

        Vector2 vel = { cosf(angle) * speed, sinf(angle) * speed };
        SpawnAsteroid(ctx, ASTEROID_LARGE, pos, vel);
    }
}

void AsteroidsSplit(GameContext *ctx, int index)
{
    Asteroid *asteroid = &ctx->asteroids[index];
    Vector2 pos = asteroid->position;
    Vector2 vel = asteroid->velocity;
    AsteroidSize size = asteroid->size;
    asteroid->active = false;

    int particleCount;
    switch (size) {
        case ASTEROID_LARGE: particleCount = 25; break;
        case ASTEROID_MEDIUM: particleCount = 14; break;
        default: particleCount = 7; break;
    }

    Color colors[] = { WHITE, LIGHTGRAY, (Color){ 100, 200, 220, 255 } };
    Color particleColor = colors[GetRandomValue(0, 2)];
    ParticlesSpawn(ctx, pos, particleCount, 80.0f, 1.0f, 3.0f, particleColor);

    if (size == ASTEROID_LARGE) {
        ctx->game.screenShakeTimer = SCREEN_SHAKE_DURATION;
        ctx->game.screenShakeMagnitude = SCREEN_SHAKE_MAGNITUDE;
    } else if (size == ASTEROID_MEDIUM) {
        ctx->game.screenShakeTimer = SCREEN_SHAKE_DURATION * 0.5f;
        ctx->game.screenShakeMagnitude = SCREEN_SHAKE_MAGNITUDE * 0.5f;
    }

    if (size == ASTEROID_LARGE || size == ASTEROID_MEDIUM) {
        AsteroidSize newSize = (size == ASTEROID_LARGE) ? ASTEROID_MEDIUM : ASTEROID_SMALL;
        for (int i = 0; i < 2; i++) {
            float angleOff = (float)GetRandomValue(30, 60) *
                (i == 0 ? 1.0f : -1.0f) * DEG2RAD;
            float baseAngle = atan2f(vel.y, vel.x) + angleOff;
            float speed = Vector2Length(vel) * 1.5f;
            if (speed < ASTEROID_SPEED_MIN) speed = ASTEROID_SPEED_MIN;
            Vector2 newVel = { cosf(baseAngle) * speed, sinf(baseAngle) * speed };
            SpawnAsteroid(ctx, newSize, pos, newVel);
        }
    }
}

int AsteroidsCountActive(const GameContext *ctx)
{
    int count = 0;
    for (int i = 0; i < MAX_ASTEROIDS; i++) {
        if (ctx->asteroids[i].active) count++;
    }
    return count;
}

void AsteroidsUpdate(GameContext *ctx, float dt)
{
    for (int i = 0; i < MAX_ASTEROIDS; i++) {
        if (!ctx->asteroids[i].active) continue;

        ctx->asteroids[i].position.x += ctx->asteroids[i].velocity.x * dt;
        ctx->asteroids[i].position.y += ctx->asteroids[i].velocity.y * dt;
        ctx->asteroids[i].position = WorldWrapPosition(ctx->asteroids[i].position);
        ctx->asteroids[i].rotation += ctx->asteroids[i].rotationSpeed * dt;
    }
}

void AsteroidsDraw(const GameContext *ctx)
{
    for (int i = 0; i < MAX_ASTEROIDS; i++) {
        if (!ctx->asteroids[i].active) continue;

        const Asteroid *asteroid = &ctx->asteroids[i];
        Vector2 points[MAX_ASTEROID_VERTS + 1];
        for (int v = 0; v < asteroid->vertCount; v++) {
            float angleDeg = (360.0f / (float)asteroid->vertCount) * (float)v + asteroid->rotation;
            float rad = angleDeg * DEG2RAD;
            points[v] = (Vector2){
                asteroid->position.x + cosf(rad) * asteroid->vertDistances[v],
                asteroid->position.y + sinf(rad) * asteroid->vertDistances[v]
            };
        }
        points[asteroid->vertCount] = points[0];
        DrawNeonLineStrip(points, asteroid->vertCount + 1, LIGHTGRAY);
    }
}

#include "collisions.h"

#include "asteroids.h"
#include "particles.h"

void CollisionsUpdate(GameContext *ctx)
{
    for (int b = 0; b < MAX_BULLETS; b++) {
        if (!ctx->bullets[b].active) continue;

        for (int a = 0; a < MAX_ASTEROIDS; a++) {
            if (!ctx->asteroids[a].active) continue;

            if (CheckCollisionCircles(ctx->bullets[b].position, BULLET_RADIUS,
                                      ctx->asteroids[a].position, ctx->asteroids[a].radius)) {
                ctx->bullets[b].active = false;
                switch (ctx->asteroids[a].size) {
                    case ASTEROID_LARGE: ctx->game.score += SCORE_LARGE; break;
                    case ASTEROID_MEDIUM: ctx->game.score += SCORE_MEDIUM; break;
                    case ASTEROID_SMALL: ctx->game.score += SCORE_SMALL; break;
                }
                AsteroidsSplit(ctx, a);
                break;
            }
        }
    }

    if (ctx->ship.alive && ctx->ship.invulnTimer <= 0) {
        for (int a = 0; a < MAX_ASTEROIDS; a++) {
            if (!ctx->asteroids[a].active) continue;

            if (CheckCollisionCircles(ctx->ship.position, SHIP_SIZE * 0.5f,
                                      ctx->asteroids[a].position, ctx->asteroids[a].radius)) {
                ctx->ship.alive = false;
                ctx->ship.respawnTimer = 2.0f;
                ctx->game.lives--;
                ParticlesSpawn(ctx, ctx->ship.position, 35, 150.0f, 1.0f, 3.0f, GREEN);
                ParticlesSpawn(ctx, ctx->ship.position, 10, 80.0f, 2.0f, 4.0f, LIME);
                ctx->game.screenShakeTimer = SCREEN_SHAKE_DURATION;
                ctx->game.screenShakeMagnitude = SCREEN_SHAKE_MAGNITUDE * 1.5f;
                break;
            }
        }
    }
}

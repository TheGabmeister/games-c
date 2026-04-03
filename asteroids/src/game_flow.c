#include "game_flow.h"

#include "asteroids.h"
#include "background.h"
#include "bullets.h"
#include "collisions.h"
#include "input.h"
#include "ship.h"

#include <string.h>

static void InitGameSession(GameContext *ctx)
{
    ctx->game.score = 0;
    ctx->game.lives = INITIAL_LIVES;
    ctx->game.wave = 0;
    ctx->game.waveCleared = false;
    ctx->game.waveTimer = 0;
    ctx->game.screenShakeTimer = 0;
    ctx->game.screenShakeMagnitude = 0;
    ctx->game.nextExtraLife = EXTRA_LIFE_SCORE;
    ctx->game.gameOverTimer = 0;

    memset(ctx->bullets, 0, sizeof(ctx->bullets));
    memset(ctx->asteroids, 0, sizeof(ctx->asteroids));
    memset(ctx->particles, 0, sizeof(ctx->particles));

    ShipInit(ctx);
}

static void StartGameSession(GameContext *ctx)
{
    InitGameSession(ctx);
    ctx->game.state = STATE_PLAYING;
    ctx->game.wave = 1;
    AsteroidsSpawnWave(ctx, 3);
}

static void UpdateWaveProgression(GameContext *ctx, float dt)
{
    if (ctx->game.waveCleared) {
        ctx->game.waveTimer -= dt;
        if (ctx->game.waveTimer <= 0) {
            ctx->game.wave++;
            int count = 2 + ctx->game.wave;
            if (count > 12) count = 12;
            AsteroidsSpawnWave(ctx, count);
            ctx->game.waveCleared = false;
        }
    } else if (AsteroidsCountActive(ctx) == 0) {
        ctx->game.waveCleared = true;
        ctx->game.waveTimer = WAVE_DELAY;
    }
}

void GameFlowBoot(GameContext *ctx)
{
    memset(ctx, 0, sizeof(*ctx));
    InputSetDefault(ctx);
    BackgroundInitStars(ctx);
    ctx->game.state = STATE_TITLE;
    ctx->game.highScore = 0;
}

void GameFlowUpdateTitle(GameContext *ctx, float dt)
{
    (void)dt;
    if (InputActionPressed(ctx, ACTION_FIRE)) {
        StartGameSession(ctx);
    }
}

void GameFlowUpdatePlaying(GameContext *ctx, float dt)
{
    ShipUpdate(ctx, dt);
    BulletsUpdate(ctx, dt);
    AsteroidsUpdate(ctx, dt);
    CollisionsUpdate(ctx);
    UpdateWaveProgression(ctx, dt);

    if (ctx->game.score >= ctx->game.nextExtraLife) {
        ctx->game.lives++;
        ctx->game.nextExtraLife += EXTRA_LIFE_SCORE;
    }
}

void GameFlowUpdateGameOver(GameContext *ctx, float dt)
{
    BulletsUpdate(ctx, dt);
    AsteroidsUpdate(ctx, dt);

    ctx->game.gameOverTimer -= dt;
    if (ctx->game.gameOverTimer <= 0 && InputActionPressed(ctx, ACTION_FIRE)) {
        ctx->game.state = STATE_TITLE;
    }
}

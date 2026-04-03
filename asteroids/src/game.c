#include "asteroids.h"
#include "background.h"
#include "bullets.h"
#include "game_flow.h"
#include "game_types.h"
#include "input.h"
#include "particles.h"
#include "ship.h"
#include "ui.h"
#include "world.h"

int main(void)
{
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Asteroids");
    SetTargetFPS(60);

    GameContext ctx = { 0 };
    GameFlowBoot(&ctx);

    char actionSet = 0;

    while (!WindowShouldClose()) {
        float dt = GetFrameTime();
        ctx.gamepadIndex = 0;

        if (IsKeyPressed(KEY_TAB)) {
            actionSet = !actionSet;
            if (actionSet == 0) InputSetDefault(&ctx);
            else InputSetCursor(&ctx);
        }

        WorldUpdateScreenShake(&ctx, dt);
        ParticlesUpdate(&ctx, dt);

        switch (ctx.game.state) {
            case STATE_TITLE:
                GameFlowUpdateTitle(&ctx, dt);
                break;
            case STATE_PLAYING:
                GameFlowUpdatePlaying(&ctx, dt);
                break;
            case STATE_GAMEOVER:
                GameFlowUpdateGameOver(&ctx, dt);
                break;
        }

        BeginDrawing();
        ClearBackground(BLACK);

        Camera2D camera = WorldBuildCamera(&ctx);
        BeginMode2D(camera);

        BackgroundDrawStarfield(&ctx);

        if (ctx.game.state == STATE_PLAYING || ctx.game.state == STATE_GAMEOVER) {
            AsteroidsDraw(&ctx);
            BulletsDraw(&ctx);
            ParticlesDraw(&ctx);
            if (ctx.ship.alive) {
                ShipDraw(&ctx);
                ShipDrawThrustFlame(&ctx);
            }
        }

        EndMode2D();

        switch (ctx.game.state) {
            case STATE_PLAYING:
                UIDrawHUD(&ctx);
                break;
            case STATE_TITLE:
                UIDrawTitleScreen(&ctx);
                break;
            case STATE_GAMEOVER:
                UIDrawGameOverScreen(&ctx);
                break;
        }

        EndDrawing();
    }

    CloseWindow();
    return 0;
}

#include "raylib.h"
#include "screens.h"
#include "gameplay.h"
#include "game_renderer.h"

static GameState game;
static bool finishScreen;

void InitGameplayScreen(AppState *app)
{
    (void)app;
    finishScreen = false;
    GameInit(&game);
}

void UpdateGameplayScreen(AppState *app, float dt)
{
    GameInput input = { 0 };

    if (IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A)) input.moveX -= 1.0f;
    if (IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D)) input.moveX += 1.0f;
    input.firePressed = IsKeyPressed(KEY_SPACE);

    GameUpdate(&game, input, dt);

    if (game.score > app->highScore) app->highScore = game.score;

    if (GameShouldEnd(&game)) {
        app->lastScore = game.score;
        app->lastWave = game.wave;
        finishScreen = true;
    }
}

void DrawGameplayScreen(const AppState *app)
{
    (void)app;
    GameDraw(&game);
}

void UnloadGameplayScreen(void) { }

bool FinishGameplayScreen(void)
{
    return finishScreen;
}

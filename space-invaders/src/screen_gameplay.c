#include "raylib.h"
#include "screens.h"
#include "gameplay.h"

static GameState game;
static int finishScreen;

void InitGameplayScreen(void)
{
    finishScreen = 0;
    GameInit(&game);
}

void UpdateGameplayScreen(void)
{
    float dt = GetFrameTime();
    GameUpdate(&game, dt, &appState.highScore);

    if (GameShouldEnd(&game)) {
        appState.lastScore = game.score;
        appState.lastWave = game.wave;
        finishScreen = 1;
    }
}

void DrawGameplayScreen(void)
{
    GameDraw(&game);
}

void UnloadGameplayScreen(void) { }

int FinishGameplayScreen(void)
{
    return finishScreen;
}

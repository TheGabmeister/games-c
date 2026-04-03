#include "raylib.h"
#include "screens.h"
#include "game_types.h"
#include "drawing.h"

static int finishScreen = 0;
static float blinkTimer = 0;
static Star stars[STAR_COUNT];

void InitEndingScreen(void)
{
    finishScreen = 0;
    blinkTimer = 0;
    InitStars(stars);
}

void UpdateEndingScreen(void)
{
    float dt = GetFrameTime();
    blinkTimer += dt;
    UpdateStars(stars, dt);

    if (IsKeyPressed(KEY_ENTER))
    {
        finishScreen = 1;
    }
}

void DrawEndingScreen(void)
{
    DrawStars(stars);

    // GAME OVER title
    const char *title = "GAME OVER";
    int titleSize = 50;
    int tw = MeasureText(title, titleSize);
    int tx = SCREEN_W / 2 - tw / 2;
    int ty = 150;
    DrawText(title, tx + 2, ty + 2, titleSize, Fade((Color){ 255, 50, 50, 255 }, 0.4f));
    DrawText(title, tx, ty, titleSize, (Color){ 255, 50, 50, 255 });

    // Final score
    const char *scoreText = TextFormat("SCORE: %d", appState.lastScore);
    int sw = MeasureText(scoreText, 30);
    DrawText(scoreText, SCREEN_W / 2 - sw / 2, 280, 30, WHITE);

    // Wave reached
    const char *waveText = TextFormat("WAVE: %d", appState.lastWave);
    int ww = MeasureText(waveText, 24);
    DrawText(waveText, SCREEN_W / 2 - ww / 2, 330, 24, COL_UI_CYAN);

    // High score
    const char *hsText = TextFormat("HIGH SCORE: %d", appState.highScore);
    int hw = MeasureText(hsText, 24);
    DrawText(hsText, SCREEN_W / 2 - hw / 2, 400, 24, COL_UI_CYAN);

    // Blinking prompt
    if (fmodf(blinkTimer, 1.0f) < 0.6f) {
        const char *prompt = "PRESS ENTER TO PLAY AGAIN";
        int pw = MeasureText(prompt, 20);
        DrawText(prompt, SCREEN_W / 2 - pw / 2, 480, 20, WHITE);
    }
}

void UnloadEndingScreen(void) { }

int FinishEndingScreen(void)
{
    return finishScreen;
}

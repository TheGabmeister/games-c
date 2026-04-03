#include "raylib.h"
#include "screens.h"
#include "game_types.h"
#include "effects.h"
#include <math.h>

static bool finishScreen = false;
static float blinkTimer = 0;
static Star stars[STAR_COUNT];

void InitEndingScreen(AppState *app)
{
    (void)app;
    finishScreen = false;
    blinkTimer = 0;
    InitStars(stars);
}

void UpdateEndingScreen(AppState *app, float dt)
{
    (void)app;
    blinkTimer += dt;
    UpdateStars(stars, dt);

    if (IsKeyPressed(KEY_ENTER))
    {
        finishScreen = true;
    }
}

void DrawEndingScreen(const AppState *app)
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
    const char *scoreText = TextFormat("SCORE: %d", app->lastScore);
    int sw = MeasureText(scoreText, 30);
    DrawText(scoreText, SCREEN_W / 2 - sw / 2, 280, 30, WHITE);

    // Wave reached
    const char *waveText = TextFormat("WAVE: %d", app->lastWave);
    int ww = MeasureText(waveText, 24);
    DrawText(waveText, SCREEN_W / 2 - ww / 2, 330, 24, COL_UI_CYAN);

    // High score
    const char *hsText = TextFormat("HIGH SCORE: %d", app->highScore);
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

bool FinishEndingScreen(void)
{
    return finishScreen;
}

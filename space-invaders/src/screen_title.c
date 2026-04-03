#include "raylib.h"
#include "screens.h"
#include "game_types.h"
#include "drawing.h"
#include "effects.h"
#include <math.h>

static bool finishScreen = false;
static float blinkTimer = 0;
static float animTimer = 0;
static int animFrame = 0;
static Star stars[STAR_COUNT];

void InitTitleScreen(AppState *app)
{
    (void)app;
    finishScreen = false;
    blinkTimer = 0;
    animTimer = 0;
    animFrame = 0;
    InitStars(stars);
}

void UpdateTitleScreen(AppState *app, float dt)
{
    (void)app;
    blinkTimer += dt;
    animTimer += dt;
    if (animTimer > 0.5f) { animTimer -= 0.5f; animFrame = 1 - animFrame; }
    UpdateStars(stars, dt);

    if (IsKeyPressed(KEY_ENTER))
    {
        finishScreen = true;
    }
}

void DrawTitleScreen(const AppState *app)
{
    DrawStars(stars);

    // Title glow + solid
    const char *title = "SPACE INVADERS";
    int titleSize = 50;
    int tw = MeasureText(title, titleSize);
    int tx = SCREEN_W / 2 - tw / 2;
    int ty = 100;
    DrawText(title, tx + 2, ty + 2, titleSize, Fade(COL_UI_CYAN, 0.4f));
    DrawText(title, tx, ty, titleSize, WHITE);

    // Alien type display
    float displayX = SCREEN_W / 2.0f - 60;
    float textX = displayX + 30;

    DrawAlienShape(AT_TOP, displayX, 240, animFrame);
    DrawText("= 30 PTS", (int)textX, 234, 20, COL_ALIEN_TOP);

    DrawAlienShape(AT_MID, displayX, 290, animFrame);
    DrawText("= 20 PTS", (int)textX, 284, 20, COL_ALIEN_MID);

    DrawAlienShape(AT_BOT, displayX, 340, animFrame);
    DrawText("= 10 PTS", (int)textX, 334, 20, COL_ALIEN_BOT);

    // Blinking prompt
    if (fmodf(blinkTimer, 1.0f) < 0.6f) {
        const char *prompt = "PRESS ENTER TO START";
        int pw = MeasureText(prompt, 20);
        DrawText(prompt, SCREEN_W / 2 - pw / 2, 440, 20, WHITE);
    }

    // High score
    if (app->highScore > 0) {
        const char *hs = TextFormat("HIGH SCORE: %d", app->highScore);
        int hw = MeasureText(hs, 20);
        DrawText(hs, SCREEN_W / 2 - hw / 2, 520, 20, COL_UI_CYAN);
    }
}

void UnloadTitleScreen(void) { }

bool FinishTitleScreen(void)
{
    return finishScreen;
}

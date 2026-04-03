#include "raylib.h"
#include "screens.h"
#include "game_types.h"

#if defined(PLATFORM_WEB)
    #include <emscripten/emscripten.h>
#endif

// Shared variables
GameScreen currentScreen = LOGO;
AppState appState = { 0 };

// Transition state
static float transAlpha = 0.0f;
static bool onTransition = false;
static bool transFadeOut = false;
static int transFromScreen = -1;
static GameScreen transToScreen = UNKNOWN;

// Forward declarations
static void TransitionToScreen(int screen);
static void UpdateTransition(void);
static void DrawTransition(void);
static void UpdateDrawFrame(void);

int main(void)
{
    InitWindow(SCREEN_W, SCREEN_H, "Space Invaders");
    SetTargetFPS(60);

    currentScreen = LOGO;
    InitLogoScreen();

#if defined(PLATFORM_WEB)
    emscripten_set_main_loop(UpdateDrawFrame, 60, 1);
#else
    while (!WindowShouldClose())
    {
        UpdateDrawFrame();
    }
#endif

    // Unload current screen
    switch (currentScreen)
    {
        case LOGO: UnloadLogoScreen(); break;
        case TITLE: UnloadTitleScreen(); break;
        case GAMEPLAY: UnloadGameplayScreen(); break;
        case ENDING: UnloadEndingScreen(); break;
        default: break;
    }

    CloseWindow();
    return 0;
}

static void TransitionToScreen(int screen)
{
    onTransition = true;
    transFadeOut = false;
    transFromScreen = currentScreen;
    transToScreen = screen;
    transAlpha = 0.0f;
}

static void UpdateTransition(void)
{
    if (!transFadeOut)
    {
        transAlpha += 0.05f;
        if (transAlpha > 1.01f)
        {
            transAlpha = 1.0f;

            switch (transFromScreen)
            {
                case LOGO: UnloadLogoScreen(); break;
                case TITLE: UnloadTitleScreen(); break;
                case GAMEPLAY: UnloadGameplayScreen(); break;
                case ENDING: UnloadEndingScreen(); break;
                default: break;
            }

            switch (transToScreen)
            {
                case LOGO: InitLogoScreen(); break;
                case TITLE: InitTitleScreen(); break;
                case GAMEPLAY: InitGameplayScreen(); break;
                case ENDING: InitEndingScreen(); break;
                default: break;
            }

            currentScreen = transToScreen;
            transFadeOut = true;
        }
    }
    else
    {
        transAlpha -= 0.02f;
        if (transAlpha < -0.01f)
        {
            transAlpha = 0.0f;
            transFadeOut = false;
            onTransition = false;
            transFromScreen = -1;
            transToScreen = UNKNOWN;
        }
    }
}

static void DrawTransition(void)
{
    DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), Fade(BLACK, transAlpha));
}

static void UpdateDrawFrame(void)
{
    if (!onTransition)
    {
        switch (currentScreen)
        {
            case LOGO:
            {
                UpdateLogoScreen();
                if (FinishLogoScreen()) TransitionToScreen(TITLE);
            } break;
            case TITLE:
            {
                UpdateTitleScreen();
                if (FinishTitleScreen()) TransitionToScreen(GAMEPLAY);
            } break;
            case GAMEPLAY:
            {
                UpdateGameplayScreen();
                if (FinishGameplayScreen()) TransitionToScreen(ENDING);
            } break;
            case ENDING:
            {
                UpdateEndingScreen();
                if (FinishEndingScreen()) TransitionToScreen(TITLE);
            } break;
            default: break;
        }
    }
    else UpdateTransition();

    BeginDrawing();

        ClearBackground(COLOR_BG);

        switch (currentScreen)
        {
            case LOGO: DrawLogoScreen(); break;
            case TITLE: DrawTitleScreen(); break;
            case GAMEPLAY: DrawGameplayScreen(); break;
            case ENDING: DrawEndingScreen(); break;
            default: break;
        }

        if (onTransition) DrawTransition();

    EndDrawing();
}

#include "raylib.h"
#include "screens.h"
#include "game_types.h"

#if defined(PLATFORM_WEB)
    #include <emscripten/emscripten.h>
#endif

static const float TRANSITION_TO_BLACK_TIME = 0.35f;
static const float TRANSITION_FROM_BLACK_TIME = 0.85f;

static GameScreen currentScreen = TITLE;
static AppState appState = { 0 };

// Transition state
static float transAlpha = 0.0f;
static bool onTransition = false;
static bool transFadeOut = false;
static GameScreen transFromScreen = UNKNOWN;
static GameScreen transToScreen = UNKNOWN;

// Forward declarations
static void InitScreen(GameScreen screen, AppState *app);
static void UpdateScreen(GameScreen screen, AppState *app, float dt);
static void DrawScreen(GameScreen screen, const AppState *app);
static void UnloadScreen(GameScreen screen);
static bool FinishScreen(GameScreen screen);
static void TransitionToScreen(GameScreen screen);
static void UpdateTransition(float dt);
static void DrawTransition(void);
static void UpdateDrawFrame(void);

int main(void)
{
    InitWindow(SCREEN_W, SCREEN_H, "Space Invaders");
    SetTargetFPS(60);

    appState = (AppState){ 0 };
    currentScreen = TITLE;
    InitScreen(currentScreen, &appState);

#if defined(PLATFORM_WEB)
    emscripten_set_main_loop(UpdateDrawFrame, 60, 1);
#else
    while (!WindowShouldClose())
    {
        UpdateDrawFrame();
    }
#endif

    UnloadScreen(currentScreen);

    CloseWindow();
    return 0;
}

static void InitScreen(GameScreen screen, AppState *app)
{
    switch (screen)
    {
        case TITLE: InitTitleScreen(app); break;
        case GAMEPLAY: InitGameplayScreen(app); break;
        case ENDING: InitEndingScreen(app); break;
        default: break;
    }
}

static void UpdateScreen(GameScreen screen, AppState *app, float dt)
{
    switch (screen)
    {
        case TITLE: UpdateTitleScreen(app, dt); break;
        case GAMEPLAY: UpdateGameplayScreen(app, dt); break;
        case ENDING: UpdateEndingScreen(app, dt); break;
        default: break;
    }
}

static void DrawScreen(GameScreen screen, const AppState *app)
{
    switch (screen)
    {
        case TITLE: DrawTitleScreen(app); break;
        case GAMEPLAY: DrawGameplayScreen(app); break;
        case ENDING: DrawEndingScreen(app); break;
        default: break;
    }
}

static void UnloadScreen(GameScreen screen)
{
    switch (screen)
    {
        case TITLE: UnloadTitleScreen(); break;
        case GAMEPLAY: UnloadGameplayScreen(); break;
        case ENDING: UnloadEndingScreen(); break;
        default: break;
    }
}

static bool FinishScreen(GameScreen screen)
{
    switch (screen)
    {
        case TITLE: return FinishTitleScreen();
        case GAMEPLAY: return FinishGameplayScreen();
        case ENDING: return FinishEndingScreen();
        default: return false;
    }
}

static void TransitionToScreen(GameScreen screen)
{
    onTransition = true;
    transFadeOut = false;
    transFromScreen = currentScreen;
    transToScreen = screen;
    transAlpha = 0.0f;
}

static void UpdateTransition(float dt)
{
    if (!transFadeOut)
    {
        transAlpha += dt / TRANSITION_TO_BLACK_TIME;
        if (transAlpha >= 1.0f)
        {
            transAlpha = 1.0f;
            UnloadScreen(transFromScreen);
            InitScreen(transToScreen, &appState);

            currentScreen = transToScreen;
            transFadeOut = true;
        }
    }
    else
    {
        transAlpha -= dt / TRANSITION_FROM_BLACK_TIME;
        if (transAlpha <= 0.0f)
        {
            transAlpha = 0.0f;
            transFadeOut = false;
            onTransition = false;
            transFromScreen = UNKNOWN;
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
    float dt = GetFrameTime();

    if (!onTransition)
    {
        UpdateScreen(currentScreen, &appState, dt);

        if (FinishScreen(currentScreen))
        {
            switch (currentScreen)
            {
                case TITLE: TransitionToScreen(GAMEPLAY); break;
                case GAMEPLAY: TransitionToScreen(ENDING); break;
                case ENDING: TransitionToScreen(TITLE); break;
                default: break;
            }
        }
    }
    else
    {
        UpdateTransition(dt);
    }

    BeginDrawing();

        ClearBackground(COLOR_BG);
        DrawScreen(currentScreen, &appState);

        if (onTransition) DrawTransition();

    EndDrawing();
}

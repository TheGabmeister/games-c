#ifndef SCREENS_H
#define SCREENS_H

#include <stdbool.h>

// Screen flow: TITLE -> GAMEPLAY -> ENDING
typedef enum GameScreen { UNKNOWN = -1, TITLE = 0, GAMEPLAY, ENDING } GameScreen;

// Shared app state (persists across screens within a session)
typedef struct {
    int highScore;
    int lastScore;
    int lastWave;
} AppState;

#ifdef __cplusplus
extern "C" {
#endif

// Title Screen
void InitTitleScreen(AppState *app);
void UpdateTitleScreen(AppState *app, float dt);
void DrawTitleScreen(const AppState *app);
void UnloadTitleScreen(void);
bool FinishTitleScreen(void);

// Gameplay Screen
void InitGameplayScreen(AppState *app);
void UpdateGameplayScreen(AppState *app, float dt);
void DrawGameplayScreen(const AppState *app);
void UnloadGameplayScreen(void);
bool FinishGameplayScreen(void);

// Ending Screen
void InitEndingScreen(AppState *app);
void UpdateEndingScreen(AppState *app, float dt);
void DrawEndingScreen(const AppState *app);
void UnloadEndingScreen(void);
bool FinishEndingScreen(void);

#ifdef __cplusplus
}
#endif

#endif // SCREENS_H

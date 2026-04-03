#ifndef SCREENS_H
#define SCREENS_H

#include "raylib.h"

// Screen flow: LOGO -> TITLE -> GAMEPLAY -> ENDING
typedef enum GameScreen { UNKNOWN = -1, LOGO = 0, TITLE, GAMEPLAY, ENDING } GameScreen;

// Shared app state (persists across screens within a session)
typedef struct {
    int highScore;
    int lastScore;
    int lastWave;
} AppState;

// Global variables shared between modules
extern GameScreen currentScreen;
extern AppState appState;

#ifdef __cplusplus
extern "C" {
#endif

// Logo Screen
void InitLogoScreen(void);
void UpdateLogoScreen(void);
void DrawLogoScreen(void);
void UnloadLogoScreen(void);
int FinishLogoScreen(void);

// Title Screen
void InitTitleScreen(void);
void UpdateTitleScreen(void);
void DrawTitleScreen(void);
void UnloadTitleScreen(void);
int FinishTitleScreen(void);

// Gameplay Screen
void InitGameplayScreen(void);
void UpdateGameplayScreen(void);
void DrawGameplayScreen(void);
void UnloadGameplayScreen(void);
int FinishGameplayScreen(void);

// Ending Screen
void InitEndingScreen(void);
void UpdateEndingScreen(void);
void DrawEndingScreen(void);
void UnloadEndingScreen(void);
int FinishEndingScreen(void);

#ifdef __cplusplus
}
#endif

#endif // SCREENS_H

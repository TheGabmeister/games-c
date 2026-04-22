#include "game.h"

int main(void) {
    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Pac-Man");
    SetTargetFPS(TARGET_FPS);
    InitAudioDevice();

    Game game;
    game_init(&game);
    game_load_sounds(&game);

    while (!WindowShouldClose()) {
        game_update(&game);
        game_draw(&game);
    }

    highscore_save(&game);
    game_unload_sounds(&game);
    CloseAudioDevice();
    CloseWindow();
    return 0;
}

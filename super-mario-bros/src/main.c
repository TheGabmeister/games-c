#include "game.h"

int main(void) {
    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Super Mario Bros");
    SetTargetFPS(TARGET_FPS);
    InitAudioDevice();

    Game game;
    game_init(&game);

    while (!WindowShouldClose()) {
        game_update(&game);
        game_draw(&game);
    }

    CloseAudioDevice();
    CloseWindow();
    return 0;
}

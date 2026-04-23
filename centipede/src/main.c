#include "game.h"
#include "sounds.h"

int main(void) {
    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Centipede");
    SetTargetFPS(TARGET_FPS);
    InitAudioDevice();

    Game game;
    game_init(&game);

    while (!WindowShouldClose()) {
        game_update(&game);
        game_draw(&game);
    }

    sounds_unload(&game);
    CloseAudioDevice();
    CloseWindow();
    return 0;
}

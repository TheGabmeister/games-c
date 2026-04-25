#include "game.h"
#include "assets.h"
#include "sounds.h"

int main(void) {
    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Joust");
    SetTargetFPS(TARGET_FPS);
    InitAudioDevice();

    Game game;
    game_init(&game);
    assets_load(&game);
    sounds_load(&game);

    while (!WindowShouldClose()) {
        game_update(&game);
        game_draw(&game);
    }

    sounds_unload(&game);
    assets_unload(&game);
    CloseAudioDevice();
    CloseWindow();
    return 0;
}

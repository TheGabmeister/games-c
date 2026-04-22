#include "game.h"
#include "sprites.h"
#include "sounds.h"

int main(void) {
    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Super Mario Bros");
    SetTargetFPS(TARGET_FPS);
    InitAudioDevice();

    ChangeDirectory(GetApplicationDirectory());
    sprites_load();
    sounds_load();

    Game game;
    game_init(&game);

    while (!WindowShouldClose()) {
        game_update(&game);
        game_draw(&game);
    }

    level_free(&game.level);
    sounds_unload();
    sprites_unload();
    CloseAudioDevice();
    CloseWindow();
    return 0;
}

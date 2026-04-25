#include "game.h"
#include "sounds.h"
#include "textures.h"

int main(void) {
    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Robotron 2084");
    SetTargetFPS(TARGET_FPS);
    InitAudioDevice();

    Game game;
    game_init(&game);
    textures_load(&game);
    sounds_load(&game);

    while (!WindowShouldClose()) {
        game_update(&game);
        game_draw(&game);
    }

    sounds_unload(&game);
    textures_unload(&game);
    CloseAudioDevice();
    CloseWindow();
    return 0;
}

#include "game.h"
#include "render.h"
#include "sounds.h"
#include "textures.h"

int main(void) {
    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Lode Runner");
    SetTargetFPS(TARGET_FPS);
    SetExitKey(0);
    InitAudioDevice();

    Game game;
    game_init(&game);
    sounds_load(game.sounds);
    textures_load(game.sprites);

    while (!WindowShouldClose() && !game.quit) {
        game_update(&game);
        game_draw(&game);
    }

    textures_unload(game.sprites);
    sounds_unload(game.sounds);
    game_shutdown(&game);
    CloseAudioDevice();
    CloseWindow();
    return 0;
}

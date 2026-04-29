#include "game.h"
#include "music.h"
#include "sounds.h"
#include "textures.h"

int main(void) {
    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "The Legend of Zelda");
    SetTargetFPS(TARGET_FPS);
    InitAudioDevice();

    Game game;
    game_init(&game);
    textures_load();
    sounds_load();
    music_init();

    while (!WindowShouldClose()) {
        game_update(&game);
        game_draw(&game);
    }

    music_cleanup();
    sounds_unload();
    textures_unload();
    CloseAudioDevice();
    CloseWindow();
    return 0;
}

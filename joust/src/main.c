#include "game.h"
#include "assets.h"
#include "sounds.h"
#include <string.h>

int main(void) {
    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Joust");
    SetTargetFPS(TARGET_FPS);
    InitAudioDevice();

    Game game;
    Resources res;
    memset(&res, 0, sizeof(res));
    game_init(&game);
    assets_load(&res);
    sounds_load(&res);

    while (!WindowShouldClose()) {
        game_update(&game, &res);
        game_draw(&game, &res);
    }

    sounds_unload(&res);
    assets_unload(&res);
    CloseAudioDevice();
    CloseWindow();
    return 0;
}

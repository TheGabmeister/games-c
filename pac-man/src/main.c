#include "game.h"

int main(void) {
    InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Pac-Man");
    SetTargetFPS(TARGET_FPS);

    Game game;
    game_init(&game);

    while (!WindowShouldClose()) {
        game_update(&game);
        game_draw(&game);
    }

    highscore_save(&game);
    CloseWindow();
    return 0;
}

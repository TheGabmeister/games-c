#include "game.h"
#include "sounds.h"

void game_init(Game *game) {
    sounds_load(game);
}


void game_update(Game *game) {
    float dt = GetFrameTime();


}


void game_draw(Game *game) {
    BeginDrawing();
    ClearBackground(COLOR_BG);

    EndDrawing();
}

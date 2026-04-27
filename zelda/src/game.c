#include "game.h"
#include "sounds.h"
#include "hud.h"
#include "textures.h"

void game_init(Game *game) {
    memset(game, 0, sizeof(*game));
    game->state = STATE_PLAY;

    player_init(&game->player);

    if (!screen_load(&game->current_screen, "assets/screens/07_07.txt")) {
        memset(&game->current_screen, TILE_FLOOR, sizeof(game->current_screen));
    }
}

void game_update(Game *game) {
    float dt = GetFrameTime();

    switch (game->state) {
        case STATE_PLAY:
            player_update(&game->player, &game->current_screen, dt);
            break;
        default:
            break;
    }
}

void game_draw(Game *game) {
    BeginDrawing();
    ClearBackground((Color){ 20, 24, 32, 255 });

    screen_draw(&game->current_screen);
    player_draw(&game->player);
    hud_draw(&game->player);

    EndDrawing();
}

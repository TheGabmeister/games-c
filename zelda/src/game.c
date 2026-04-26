#include "game.h"
#include "sounds.h"
#include "raymath.h"

void game_init(Game *game) {
    memset(game, 0, sizeof(*game));
    game->player_position = (Vector2){ WINDOW_WIDTH * 0.5f, WINDOW_HEIGHT * 0.5f };
    game->player_speed = 280.0f;
    game->player_radius = 24.0f;
    game->player_color = SKYBLUE;
}

void game_update(Game *game) {
    float dt = GetFrameTime();
    Vector2 move = { 0.0f, 0.0f };

    if (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT)) move.x -= 1.0f;
    if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT)) move.x += 1.0f;
    if (IsKeyDown(KEY_W) || IsKeyDown(KEY_UP)) move.y -= 1.0f;
    if (IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN)) move.y += 1.0f;

    if (move.x != 0.0f || move.y != 0.0f) {
        move = Vector2Normalize(move);
        game->player_position.x += move.x * game->player_speed * dt;
        game->player_position.y += move.y * game->player_speed * dt;
    }

    game->player_position.x = Clamp(game->player_position.x, game->player_radius, WINDOW_WIDTH - game->player_radius);
    game->player_position.y = Clamp(game->player_position.y, game->player_radius, WINDOW_HEIGHT - game->player_radius);

    if (IsKeyPressed(KEY_SPACE)) {
        sound_play(game, SOUND_COIN);
    }
}

void game_draw(Game *game) {
    BeginDrawing();
    ClearBackground((Color){ 20, 24, 32, 255 });

    DrawCircleV(game->player_position, game->player_radius, game->player_color);
    DrawText("raylib C template", 24, 24, 28, RAYWHITE);
    DrawText("Move with WASD or arrows. Space plays assets/coin.wav if present.", 24, 60, 18, LIGHTGRAY);

    EndDrawing();
}

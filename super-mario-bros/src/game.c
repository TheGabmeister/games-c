#include "game.h"

void game_init(Game *game) {
    *game = (Game){0};
    game->state = STATE_TITLE;
    game->lives = 3;
    game->level = 1;
}

static void update_title(Game *game) {
    if (IsKeyPressed(KEY_ENTER) ||
        (IsGamepadAvailable(0) && (IsGamepadButtonPressed(0, GAMEPAD_BUTTON_RIGHT_FACE_DOWN) ||
                                   IsGamepadButtonPressed(0, GAMEPAD_BUTTON_RIGHT_FACE_RIGHT)))) {
        game->state = STATE_PLAYING;
    }
}

static void update_playing(Game *game) {
    float dt = GetFrameTime();
    particles_update(game->particles, dt);
    (void)game;
}

void game_update(Game *game) {
    switch (game->state) {
        case STATE_TITLE: update_title(game); break;
        case STATE_PLAYING: update_playing(game); break;
        default: break;
    }
}

static void draw_title(Game *game) {
    (void)game;
    const char *title = "SUPER MARIO BROS";
    int title_width = MeasureText(title, 40);
    DrawText(title, (WINDOW_WIDTH - title_width) / 2, WINDOW_HEIGHT / 2 - 60, 40, COLOR_TEXT);

    const char *prompt = "Press ENTER to Start";
    int prompt_width = MeasureText(prompt, 20);
    DrawText(prompt, (WINDOW_WIDTH - prompt_width) / 2, WINDOW_HEIGHT / 2 + 20, 20, COLOR_TEXT);
}

static void draw_playing(Game *game) {
    particles_draw(game->particles);
}

static void draw_hud(Game *game) {
    DrawText(TextFormat("SCORE: %d", game->score), 10, 10, 20, COLOR_TEXT);
    DrawText(TextFormat("COINS: %d", game->coins), 300, 10, 20, COLOR_TEXT);
    DrawText(TextFormat("WORLD %d", game->level), 550, 10, 20, COLOR_TEXT);
    DrawText(TextFormat("LIVES: %d", game->lives), 700, 10, 20, COLOR_TEXT);
}

void game_draw(Game *game) {
    BeginDrawing();
    ClearBackground(COLOR_BG);

    switch (game->state) {
        case STATE_TITLE:
            draw_title(game);
            break;
        case STATE_PLAYING:
            draw_hud(game);
            draw_playing(game);
            break;
        default:
            break;
    }

    EndDrawing();
}

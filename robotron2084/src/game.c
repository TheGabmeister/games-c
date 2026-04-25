#include "game.h"
#include "sounds.h"
#include "world.h"

static void start_new_game(Game *game) {
    game->score = 0;
    game->lives = PLAYER_LIVES;
    game->wave = 1;
    game->next_extra_life_score = EXTRA_LIFE_SCORE;
    game->humans_rescued_this_wave = 0;
    game->mode = GAME_MODE_PLAYING;
    sound_play(game, SOUND_START);
    world_spawn_wave(game);
}

void game_init(Game *game) {
    memset(game, 0, sizeof(*game));
    game->mode = GAME_MODE_TITLE;
    game->lives = PLAYER_LIVES;
    game->wave = 1;
    game->next_extra_life_score = EXTRA_LIFE_SCORE;
    game->player_speed = PLAYER_SPEED;
    game->player_radius = PLAYER_RADIUS;
    game->player_color = SKYBLUE;
    world_reset_player(game);
}

void game_update(Game *game) {
    float dt = GetFrameTime();

    if (game->screen_shake > 0.0f) {
        game->screen_shake -= dt;
    }

    if (game->screen_flash > 0.0f) {
        game->screen_flash -= dt;
    }

    if ((game->mode == GAME_MODE_TITLE || game->mode == GAME_MODE_GAME_OVER) && IsKeyPressed(KEY_ENTER)) {
        start_new_game(game);
    }

    if (game->mode == GAME_MODE_PLAYING && IsKeyPressed(KEY_P)) {
        game->mode = GAME_MODE_PAUSED;
    } else if (game->mode == GAME_MODE_PAUSED && IsKeyPressed(KEY_P)) {
        game->mode = GAME_MODE_PLAYING;
    }

    if (game->mode == GAME_MODE_PLAYING) {
        world_update_playing(game, dt);
    }
}

static void draw_overlay(Game *game) {
    DrawText("ROBOTRON 2084 - SPAWNER ENEMIES", 24, 24, 24, RAYWHITE);
    DrawText(TextFormat("SCORE %06d   HIGH %06d   WAVE %d   LIVES %d   G %d   S %d   E %d   Q %d   T %d   SHOTS %d",
        game->score,
        game->high_score,
        game->wave,
        game->lives,
        world_count_active_grunts(game),
        world_count_active_spheroids(game),
        world_count_active_enforcers(game),
        world_count_active_quarks(game),
        world_count_active_tanks(game),
        world_count_active_projectiles(game)), 24, 56, 18, LIGHTGRAY);
    DrawText(TextFormat("WASD move  |  Arrow keys fire  |  P pause  |  Humans %d  Hulks %d  Electrodes %d  Next rescue +%d",
        world_count_active_humans(game),
        world_count_active_hulks(game),
        world_count_active_electrodes(game),
        world_next_human_rescue_score(game)), 24, 82, 18, GRAY);
}

void game_draw(Game *game) {
    BeginDrawing();
    ClearBackground((Color){ 10, 12, 18, 255 });

    Vector2 shake = { 0.0f, 0.0f };
    if (game->screen_shake > 0.0f) {
        float amount = 10.0f * (game->screen_shake / 0.28f);
        shake.x = (float)GetRandomValue(-(int)amount, (int)amount);
        shake.y = (float)GetRandomValue(-(int)amount, (int)amount);
    }

    Camera2D camera = { 0 };
    camera.offset = shake;
    camera.target = (Vector2){ 0.0f, 0.0f };
    camera.rotation = 0.0f;
    camera.zoom = 1.0f;

    BeginMode2D(camera);
    world_draw_playfield(game);
    EndMode2D();

    draw_overlay(game);

    if (game->mode == GAME_MODE_TITLE) {
        DrawRectangle(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, (Color){ 0, 0, 0, 150 });
        DrawText("ROBOTRON 2084", 396, 360, 56, RAYWHITE);
        DrawText("Press Enter", 520, 430, 28, SKYBLUE);
    } else if (game->mode == GAME_MODE_PAUSED) {
        DrawRectangle(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, (Color){ 0, 0, 0, 150 });
        DrawText("PAUSED", 514, 400, 48, RAYWHITE);
        DrawText("Press P to resume", 494, 460, 22, LIGHTGRAY);
    } else if (game->mode == GAME_MODE_GAME_OVER) {
        DrawRectangle(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, (Color){ 0, 0, 0, 170 });
        DrawText("GAME OVER", 460, 390, 48, RAYWHITE);
        DrawText(TextFormat("Final score: %d", game->score), 500, 445, 24, LIGHTGRAY);
        DrawText("Press Enter", 520, 485, 24, SKYBLUE);
    }

    if (game->screen_flash > 0.0f) {
        float alpha = Clamp(game->screen_flash / 0.18f, 0.0f, 1.0f);
        DrawRectangle(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, Fade(RAYWHITE, alpha * 0.22f));
    }

    EndDrawing();
}

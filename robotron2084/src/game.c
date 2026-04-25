#include "game.h"
#include "sounds.h"
#include "world.h"
#include <stdio.h>

static const char *HIGH_SCORE_FILE = "highscore.txt";

static void load_high_score(Game *game) {
    if (!FileExists(HIGH_SCORE_FILE)) {
        return;
    }

    char *text = LoadFileText(HIGH_SCORE_FILE);
    if (text) {
        int value = atoi(text);
        if (value > 0) {
            game->high_score = value;
        }
        UnloadFileText(text);
    }
}

static void save_high_score(Game *game) {
    if (!game->high_score_dirty) {
        return;
    }

    char buffer[32];
    snprintf(buffer, sizeof(buffer), "%d\n", game->high_score);
    if (SaveFileText(HIGH_SCORE_FILE, buffer)) {
        game->high_score_dirty = false;
    }
}

static void start_new_game(Game *game) {
    game->score = 0;
    game->lives = PLAYER_LIVES;
    game->wave = 1;
    game->next_extra_life_score = EXTRA_LIFE_SCORE;
    game->humans_rescued_this_wave = 0;
    sound_play(game, SOUND_START);
    world_spawn_wave(game);
    game->mode = GAME_MODE_WAVE_INTRO;
    game->mode_timer = WAVE_INTRO_TIME;
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
    load_high_score(game);
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

    save_high_score(game);

    if ((game->mode == GAME_MODE_TITLE || game->mode == GAME_MODE_GAME_OVER) && IsKeyPressed(KEY_ENTER)) {
        start_new_game(game);
    }

    if ((game->mode == GAME_MODE_PLAYING || game->mode == GAME_MODE_WAVE_INTRO) && IsKeyPressed(KEY_P)) {
        game->mode = GAME_MODE_PAUSED;
    } else if (game->mode == GAME_MODE_PAUSED && IsKeyPressed(KEY_P)) {
        game->mode = GAME_MODE_PLAYING;
    }

    if (game->mode == GAME_MODE_PLAYING) {
        world_update_playing(game, dt);
    } else if (game->mode == GAME_MODE_WAVE_INTRO) {
        game->mode_timer -= dt;
        if (game->mode_timer <= 0.0f) {
            game->mode = GAME_MODE_PLAYING;
        }
    } else if (game->mode == GAME_MODE_PLAYER_DEAD) {
        game->mode_timer -= dt;
        if (game->mode_timer <= 0.0f) {
            world_reset_player(game);
            game->mode = GAME_MODE_PLAYING;
        }
    }
}

void game_shutdown(Game *game) {
    save_high_score(game);
}

static void draw_overlay(Game *game) {
    DrawRectangle(0, 0, WINDOW_WIDTH, 104, (Color){ 4, 6, 12, 185 });
    DrawText("ROBOTRON 2084", 24, 18, 24, RAYWHITE);
    DrawText(TextFormat("SCORE %06d   HIGH %06d   WAVE %03d", game->score, game->high_score, game->wave),
        24, 50, 20, LIGHTGRAY);

    for (int i = 0; i < game->lives; i++) {
        Vector2 center = { WINDOW_WIDTH - 36.0f - (float)i * 26.0f, 30.0f };
        DrawCircleV(center, 9.0f, SKYBLUE);
        DrawCircleV(center, 3.0f, RAYWHITE);
    }

    DrawText(TextFormat("G %d  S %d  E %d  Q %d  T %d  B %d  P %d  SHOTS %d",
        world_count_active_grunts(game),
        world_count_active_spheroids(game),
        world_count_active_enforcers(game),
        world_count_active_quarks(game),
        world_count_active_tanks(game),
        world_count_active_brains(game),
        world_count_active_progs(game),
        world_count_active_projectiles(game)), 24, 76, 16, GRAY);
    DrawText(TextFormat("HUMANS %d   HULKS %d   ELECTRODES %d   NEXT RESCUE +%d",
        world_count_active_humans(game),
        world_count_active_hulks(game),
        world_count_active_electrodes(game),
        world_next_human_rescue_score(game)), 642, 76, 16, GRAY);
}

static void draw_centered_text(const char *text, int y, int font_size, Color color) {
    DrawText(text, WINDOW_WIDTH / 2 - MeasureText(text, font_size) / 2, y, font_size, color);
}

static void draw_vignette(unsigned char alpha) {
    DrawRectangle(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, (Color){ 0, 0, 0, alpha });
    DrawRectangleLinesEx((Rectangle){ 18.0f, 18.0f, WINDOW_WIDTH - 36.0f, WINDOW_HEIGHT - 36.0f }, 2.0f, (Color){ 77, 214, 255, 80 });
}

void game_draw(Game *game) {
    BeginDrawing();
    ClearBackground((Color){ 10, 12, 18, 255 });

    Vector2 shake = { 0.0f, 0.0f };
    if (game->screen_shake > 0.0f) {
        float amount = 10.0f * (game->screen_shake / SCREEN_SHAKE_DURATION);
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
        draw_vignette(175);
        draw_centered_text("ROBOTRON 2084", 330, 58, RAYWHITE);
        draw_centered_text(TextFormat("HIGH SCORE %06d", game->high_score), 402, 24, GOLD);
        draw_centered_text("WASD MOVE   ARROWS FIRE   P PAUSE", 458, 20, LIGHTGRAY);
        draw_centered_text("PRESS ENTER", 512, 28, SKYBLUE);
    } else if (game->mode == GAME_MODE_WAVE_INTRO) {
        DrawRectangle(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, (Color){ 0, 0, 0, 105 });
        draw_centered_text(TextFormat("WAVE %d", game->wave), 384, 54, RAYWHITE);
        draw_centered_text("GET READY", 450, 24, SKYBLUE);
    } else if (game->mode == GAME_MODE_PLAYER_DEAD) {
        DrawRectangle(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, (Color){ 0, 0, 0, 120 });
        draw_centered_text("PLAYER DOWN", 390, 42, RAYWHITE);
        draw_centered_text(TextFormat("LIVES %d", game->lives), 446, 24, SKYBLUE);
    } else if (game->mode == GAME_MODE_PAUSED) {
        draw_vignette(150);
        draw_centered_text("PAUSED", 392, 48, RAYWHITE);
        draw_centered_text("PRESS P TO RESUME", 456, 22, LIGHTGRAY);
    } else if (game->mode == GAME_MODE_GAME_OVER) {
        draw_vignette(180);
        draw_centered_text("GAME OVER", 360, 52, RAYWHITE);
        draw_centered_text(TextFormat("FINAL SCORE %06d", game->score), 430, 24, LIGHTGRAY);
        draw_centered_text(TextFormat("HIGH SCORE %06d", game->high_score), 466, 24, GOLD);
        draw_centered_text("PRESS ENTER", 522, 26, SKYBLUE);
    }

    if (game->screen_flash > 0.0f) {
        float alpha = Clamp(game->screen_flash / SCREEN_FLASH_DURATION, 0.0f, 1.0f);
        DrawRectangle(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, Fade(RAYWHITE, alpha * 0.22f));
    }

    EndDrawing();
}

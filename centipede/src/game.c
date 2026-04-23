#include "game.h"
#include "player.h"
#include "mushroom.h"
#include "centipede.h"
#include "enemies.h"
#include "sounds.h"

static void game_start_level(Game *game);
static void game_player_die(Game *game);
static void check_extra_life(Game *game);
static void collide_dart_mushrooms(Game *game);
static void collide_dart_segments(Game *game);
static void collide_dart_spider(Game *game);
static void collide_dart_flea(Game *game);
static void collide_dart_scorpion(Game *game);
static void collide_segments_player(Game *game);
static void collide_spider_player(Game *game);
static void update_restoration(Game *game, float dt);
static void draw_hud(Game *game);

void game_init(Game *game) {
    memset(game, 0, sizeof(Game));
    game->state = STATE_TITLE;
    game->lives = PLAYER_LIVES;
    game->level = 1;
    game->next_extra_life = EXTRA_LIFE_INTERVAL;
    game->centipede_speed = CENTIPEDE_BASE_SPEED;
    player_init(&game->player);
    mushroom_init(&game->mushrooms);
    sounds_load(game);
}

static void game_start_level(Game *game) {
    game->centipede_speed = CENTIPEDE_BASE_SPEED + (game->level - 1) * 15.0f;
    if (game->centipede_speed > 250.0f) game->centipede_speed = 250.0f;

    centipede_init(game->segments, game->level);

    game->spider.active = false;
    game->flea.active = false;
    game->scorpion.active = false;
    game->spider_spawn_timer = 0;
    game->scorpion_spawn_timer = 0;

    game->dart.active = false;
    game->player.alive = true;

    game->state = STATE_READY;
    game->state_timer = READY_DURATION;
}

static void game_player_die(Game *game) {
    game->player.alive = false;
    game->lives--;
    game->dart.active = false;
    game->spider.active = false;
    game->flea.active = false;
    game->scorpion.active = false;
    game->state = STATE_DYING;
    game->state_timer = DYING_DURATION;
    sound_play(game, SND_PLAYER_DEATH);
}

static void check_extra_life(Game *game) {
    if (game->score >= game->next_extra_life) {
        game->lives++;
        game->next_extra_life += EXTRA_LIFE_INTERVAL;
        sound_play(game, SND_EXTRA_LIFE);
    }
}

// --- Collision helpers ---

static void collide_dart_mushrooms(Game *game) {
    if (!game->dart.active) return;
    int dc = px_to_col(game->dart.x + DART_WIDTH / 2);
    int dr = px_to_row(game->dart.y);
    if (dc < 0 || dc >= GRID_COLS || dr < 0 || dr >= GRID_ROWS) return;
    if (MUSH_IS_PRESENT(game->mushrooms.cells[dr][dc])) {
        int pts = mushroom_hit(&game->mushrooms, dc, dr);
        game->score += pts;
        game->dart.active = false;
        sound_play(game, SND_MUSHROOM_HIT);
    }
}

static void collide_dart_segments(Game *game) {
    if (!game->dart.active) return;
    int dc = px_to_col(game->dart.x + DART_WIDTH / 2);
    int dr = px_to_row(game->dart.y);

    for (int i = 0; i < MAX_SEGMENTS; i++) {
        if (!game->segments[i].active) continue;
        if (game->segments[i].col < 0) continue;
        if (game->segments[i].col == dc && game->segments[i].row == dr) {
            bool was_head = centipede_hit_segment(game->segments, i, &game->mushrooms);
            game->score += was_head ? HEAD_POINTS : BODY_POINTS;
            game->dart.active = false;
            check_extra_life(game);
            sound_play(game, SND_SEGMENT_HIT);
            return;
        }
    }
}

static void collide_dart_spider(Game *game) {
    if (!game->dart.active || !game->spider.active) return;
    Rectangle sr = { game->spider.x, game->spider.y, CELL_SIZE, CELL_SIZE };
    Rectangle dr = { game->dart.x, game->dart.y, DART_WIDTH, DART_HEIGHT };
    if (CheckCollisionRecs(sr, dr)) {
        game->spider.active = false;
        game->dart.active = false;

        int dist_rows = game->player.row - px_to_row(game->spider.y + CELL_SIZE / 2);
        if (dist_rows < 0) dist_rows = -dist_rows;
        int pts;
        if (dist_rows <= SPIDER_CLOSE_DIST) pts = SPIDER_POINTS_CLOSE;
        else if (dist_rows <= SPIDER_MID_DIST) pts = SPIDER_POINTS_MID;
        else pts = SPIDER_POINTS_FAR;
        game->score += pts;
        check_extra_life(game);
        sound_play(game, SND_SPIDER);
    }
}

static void collide_dart_flea(Game *game) {
    if (!game->dart.active || !game->flea.active) return;
    Rectangle fr = { game->flea.x, game->flea.y, CELL_SIZE, CELL_SIZE };
    Rectangle dr = { game->dart.x, game->dart.y, DART_WIDTH, DART_HEIGHT };
    if (CheckCollisionRecs(fr, dr)) {
        game->dart.active = false;
        game->flea.hp--;
        if (game->flea.hp <= 0) {
            game->flea.active = false;
            game->score += FLEA_POINTS;
            check_extra_life(game);
        } else {
            game->flea.fast = true;
        }
        sound_play(game, SND_FLEA);
    }
}

static void collide_dart_scorpion(Game *game) {
    if (!game->dart.active || !game->scorpion.active) return;
    Rectangle sr = { game->scorpion.x, game->scorpion.y, CELL_SIZE, CELL_SIZE };
    Rectangle dr = { game->dart.x, game->dart.y, DART_WIDTH, DART_HEIGHT };
    if (CheckCollisionRecs(sr, dr)) {
        game->scorpion.active = false;
        game->dart.active = false;
        game->score += SCORPION_POINTS;
        check_extra_life(game);
        sound_play(game, SND_SCORPION);
    }
}

static void collide_segments_player(Game *game) {
    if (!game->player.alive) return;
    for (int i = 0; i < MAX_SEGMENTS; i++) {
        if (!game->segments[i].active) continue;
        if (game->segments[i].col < 0) continue;
        if (game->segments[i].col == game->player.col &&
            game->segments[i].row == game->player.row) {
            game_player_die(game);
            return;
        }
    }
}

static void collide_spider_player(Game *game) {
    if (!game->player.alive || !game->spider.active) return;
    Rectangle sr = { game->spider.x, game->spider.y, CELL_SIZE, CELL_SIZE };
    Rectangle pr = { game->player.x, game->player.y, CELL_SIZE, CELL_SIZE };
    if (CheckCollisionRecs(sr, pr)) {
        game_player_die(game);
    }
}

static void update_restoration(Game *game, float dt) {
    game->restore_timer += dt;
    while (game->restore_timer >= RESTORE_INTERVAL) {
        game->restore_timer -= RESTORE_INTERVAL;

        bool found = false;
        while (game->restore_row < GRID_ROWS) {
            int r = game->restore_row;
            int c = game->restore_col;
            game->restore_col++;
            if (game->restore_col >= GRID_COLS) {
                game->restore_col = 0;
                game->restore_row++;
            }

            unsigned char v = game->mushrooms.cells[r][c];
            if (MUSH_IS_PRESENT(v)) {
                int hp = MUSH_HP(v);
                if (hp < MUSHROOM_MAX_HP || MUSH_IS_POISONED(v)) {
                    game->mushrooms.cells[r][c] = MUSH_MAKE(MUSHROOM_MAX_HP);
                    game->score += MUSHROOM_RESTORE_PTS;
                    found = true;
                    break;
                }
            }
        }

        if (!found && game->restore_row >= GRID_ROWS) {
            if (game->lives > 0) {
                player_init(&game->player);
                centipede_init(game->segments, game->level);
                game->state = STATE_READY;
                game->state_timer = READY_DURATION;
            } else {
                game->state = STATE_GAME_OVER;
                game->state_timer = GAME_OVER_DURATION;
                if (game->score > game->high_score) {
                    game->high_score = game->score;
                }
            }
            return;
        }
    }
}

// --- Main update ---

void game_update(Game *game) {
    float dt = GetFrameTime();

    switch (game->state) {
    case STATE_TITLE:
        if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE)) {
            game->score = 0;
            game->lives = PLAYER_LIVES;
            game->level = 1;
            game->next_extra_life = EXTRA_LIFE_INTERVAL;
            player_init(&game->player);
            mushroom_init(&game->mushrooms);
            game_start_level(game);
        }
        break;

    case STATE_READY:
        game->state_timer -= dt;
        if (game->state_timer <= 0) {
            game->state = STATE_PLAYING;
        }
        break;

    case STATE_PLAYING: {
        bool dart_was_active = game->dart.active;
        player_update(&game->player, &game->dart, &game->mushrooms, dt);
        if (!dart_was_active && game->dart.active) {
            sound_play(game, SND_SHOOT);
        }
        dart_update(&game->dart, dt);
        centipede_update(game->segments, &game->mushrooms, game->centipede_speed, dt);

        spider_try_spawn(&game->spider, &game->spider_spawn_timer, game->level, dt);
        spider_update(&game->spider, &game->mushrooms, dt);

        flea_try_spawn(&game->flea, &game->mushrooms);
        flea_update(&game->flea, &game->mushrooms, dt);

        scorpion_try_spawn(&game->scorpion, &game->scorpion_spawn_timer, game->level, dt);
        scorpion_update(&game->scorpion, &game->mushrooms, dt);

        collide_dart_mushrooms(game);
        collide_dart_segments(game);
        collide_dart_spider(game);
        collide_dart_flea(game);
        collide_dart_scorpion(game);
        collide_segments_player(game);
        collide_spider_player(game);

        if (centipede_active_count(game->segments) == 0) {
            game->level++;
            game->state = STATE_LEVEL_COMPLETE;
            game->state_timer = LEVEL_COMPLETE_DURATION;
            sound_play(game, SND_LEVEL_COMPLETE);
        }
        break;
    }

    case STATE_DYING:
        game->state_timer -= dt;
        if (game->state_timer <= 0) {
            game->state = STATE_RESTORING;
            game->restore_row = 0;
            game->restore_col = 0;
            game->restore_timer = 0;
        }
        break;

    case STATE_RESTORING:
        update_restoration(game, dt);
        break;

    case STATE_LEVEL_COMPLETE:
        game->state_timer -= dt;
        if (game->state_timer <= 0) {
            game_start_level(game);
        }
        break;

    case STATE_GAME_OVER:
        game->state_timer -= dt;
        if (game->state_timer <= 0 || IsKeyPressed(KEY_ENTER)) {
            game->state = STATE_TITLE;
        }
        break;
    }
}

// --- HUD ---

static void draw_hud(Game *game) {
    DrawText(TextFormat("SCORE %d", game->score), 10, 10, 20, COLOR_HUD);
    DrawText(TextFormat("LEVEL %d", game->level), WINDOW_WIDTH / 2 - 40, 10, 20, COLOR_HUD);

    for (int i = 0; i < game->lives; i++) {
        int lx = WINDOW_WIDTH - 30 - i * 22;
        Vector2 v1 = { (float)lx + 8, 8 };
        Vector2 v2 = { (float)lx, 30 };
        Vector2 v3 = { (float)lx + 16, 30 };
        DrawTriangle(v1, v2, v3, COLOR_PLAYER);
    }

    if (game->high_score > 0) {
        DrawText(TextFormat("HI %d", game->high_score),
                 WINDOW_WIDTH / 2 - 120, 10, 20, YELLOW);
    }
}

// --- Main draw ---

void game_draw(Game *game) {
    BeginDrawing();
    ClearBackground(COLOR_BG);

    switch (game->state) {
    case STATE_TITLE: {
        const char *title = "CENTIPEDE";
        int tw = MeasureText(title, 50);
        DrawText(title, WINDOW_WIDTH / 2 - tw / 2, WINDOW_HEIGHT / 2 - 80, 50, GREEN);

        if (game->high_score > 0) {
            const char *hi = TextFormat("HIGH SCORE: %d", game->high_score);
            int hw = MeasureText(hi, 20);
            DrawText(hi, WINDOW_WIDTH / 2 - hw / 2, WINDOW_HEIGHT / 2 - 10, 20, YELLOW);
        }

        float blink = GetTime();
        if ((int)(blink * 2) % 2 == 0) {
            const char *start = "PRESS ENTER TO START";
            int sw = MeasureText(start, 20);
            DrawText(start, WINDOW_WIDTH / 2 - sw / 2, WINDOW_HEIGHT / 2 + 40, 20, WHITE);
        }
        break;
    }

    case STATE_READY:
        draw_hud(game);
        mushroom_draw(&game->mushrooms);
        player_draw(&game->player);
        {
            const char *ready = "READY";
            int rw = MeasureText(ready, 40);
            DrawText(ready, WINDOW_WIDTH / 2 - rw / 2, WINDOW_HEIGHT / 2 - 20, 40, YELLOW);
        }
        break;

    case STATE_PLAYING:
    case STATE_DYING:
    case STATE_RESTORING:
        draw_hud(game);
        mushroom_draw(&game->mushrooms);
        centipede_draw(game->segments);
        spider_draw(&game->spider);
        flea_draw(&game->flea);
        scorpion_draw(&game->scorpion);
        player_draw(&game->player);
        dart_draw(&game->dart);

        if (game->state == STATE_DYING) {
            float alpha = game->state_timer / DYING_DURATION;
            DrawCircle((int)(game->player.x + CELL_SIZE / 2),
                       (int)(game->player.y + CELL_SIZE / 2),
                       CELL_SIZE * (1.0f + (1.0f - alpha) * 2.0f),
                       (Color){255, 255, 255, (unsigned char)(alpha * 200)});
        }
        break;

    case STATE_LEVEL_COMPLETE:
        draw_hud(game);
        mushroom_draw(&game->mushrooms);
        player_draw(&game->player);
        {
            const char *lc = TextFormat("LEVEL %d COMPLETE", game->level - 1);
            int lcw = MeasureText(lc, 30);
            DrawText(lc, WINDOW_WIDTH / 2 - lcw / 2, WINDOW_HEIGHT / 2 - 15, 30, GREEN);
        }
        break;

    case STATE_GAME_OVER: {
        draw_hud(game);
        const char *go = "GAME OVER";
        int gow = MeasureText(go, 50);
        DrawText(go, WINDOW_WIDTH / 2 - gow / 2, WINDOW_HEIGHT / 2 - 60, 50, RED);

        const char *fs = TextFormat("FINAL SCORE: %d", game->score);
        int fsw = MeasureText(fs, 25);
        DrawText(fs, WINDOW_WIDTH / 2 - fsw / 2, WINDOW_HEIGHT / 2 + 10, 25, WHITE);

        const char *restart = "PRESS ENTER";
        int rsw = MeasureText(restart, 20);
        DrawText(restart, WINDOW_WIDTH / 2 - rsw / 2, WINDOW_HEIGHT / 2 + 60, 20, GRAY);
        break;
    }
    }

    EndDrawing();
}

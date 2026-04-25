#include "render.h"
#include "particles.h"
#include "textures.h"

#include <math.h>

/* ——— Sprite helpers ——— */

static void draw_sprite(Texture2D tex, int x, int y, Color tint) {
    if (tex.id != 0) {
        DrawTexture(tex, x, y, tint);
    }
}

static void draw_sprite_flipped(Texture2D tex, float cx, float cy, Dir facing, Color tint) {
    if (tex.id == 0) return;
    float w = (float)tex.width;
    float h = (float)tex.height;
    Rectangle src = { 0, 0, facing == DIR_RIGHT ? w : -w, h };
    Rectangle dst = { cx - w * 0.5f, cy - h * 0.5f, w, h };
    DrawTexturePro(tex, src, dst, (Vector2){ 0, 0 }, 0.0f, tint);
}

/* ——— Tile rendering ——— */

static TextureID tex_for_tile(TileID tile) {
    switch (tile) {
        case TILE_BRICK:        return TEX_BRICK;
        case TILE_SOLID:        return TEX_SOLID;
        case TILE_LADDER:       return TEX_LADDER;
        case TILE_ROPE:         return TEX_ROPE;
        case TILE_GOLD:         return TEX_GOLD;
        case TILE_EXIT_LADDER:  return TEX_EXIT_LADDER;
        case TILE_TRAPDOOR:     return TEX_TRAPDOOR;
        case TILE_HOLE:         return TEX_HOLE;
        default:                return TEX_COUNT;
    }
}

static void draw_tile(TileID tile, int r, int c, bool exit_revealed, float exit_alpha,
                       float hole_timer, const Texture2D sprites[TEX_COUNT]) {
    int x = PLAY_OFFSET_X + c * TILE_SIZE;
    int y = PLAY_OFFSET_Y + r * TILE_SIZE;

    if (tile == TILE_EXIT_LADDER && !exit_revealed) {
        return;
    }

    TextureID tex_id = tex_for_tile(tile);
    if (tex_id >= TEX_COUNT) return;

    Color tint = WHITE;
    if (tile == TILE_EXIT_LADDER) {
        tint.a = (unsigned char)(255.0f * exit_alpha);
    }

    draw_sprite(sprites[tex_id], x, y, tint);

    if (tile == TILE_GOLD) {
        Vector2 center = { (float)x + TILE_SIZE * 0.5f, (float)y + TILE_SIZE * 0.5f };
        float sparkle = 0.5f + 0.5f * sinf((float)GetTime() * 4.0f + (float)(r * 7 + c * 13));
        DrawCircleV((Vector2){ center.x - 3, center.y - 3 },
                    2.0f + sparkle,
                    (Color){ 255, 239, 146, (unsigned char)(180.0f * sparkle) });
    } else if (tile == TILE_HOLE) {
        Rectangle rect = { (float)x, (float)y, (float)TILE_SIZE, (float)TILE_SIZE };
        Color rim = (Color){ 77, 51, 40, 0 };
        if (hole_timer <= DIG_REFILL_SEC - DIG_WARN_FLASH_AT) {
            rim = RED;
        } else if (hole_timer <= DIG_REFILL_SEC - DIG_WARN_PULSE_AT) {
            rim = ORANGE;
        }
        if (rim.a > 0) {
            DrawRectangleLinesEx(rect, 2.0f, rim);
        }
    }
}

/* ——— World rendering ——— */

static void draw_world(const Game *game, float exit_alpha) {
    DrawRectangle(PLAY_OFFSET_X, PLAY_OFFSET_Y,
                  GRID_COLS * TILE_SIZE, GRID_ROWS * TILE_SIZE,
                  (Color){ 15, 17, 23, 255 });

    for (int r = 0; r < GRID_ROWS; r++) {
        for (int c = 0; c < GRID_COLS; c++) {
            draw_tile(game->world.tiles[r][c], r, c,
                      game->world.all_gold_collected, exit_alpha,
                      game->world.hole_timer[r][c], game->sprites);
        }
    }
}

/* ——— Parallax background ——— */

static void draw_parallax(const Game *game) {
    if (game->bg_texture.id == 0) return;
    int tw = game->bg_texture.width;
    int th = game->bg_texture.height;
    int scroll_y = (int)(GetTime() * 8.0) % th;
    Color tint = { 255, 255, 255, 76 };

    for (int y = HUD_HEIGHT - scroll_y; y < WINDOW_HEIGHT; y += th) {
        for (int x = 0; x < WINDOW_WIDTH; x += tw) {
            DrawTexture(game->bg_texture, x, y, tint);
        }
    }
}

/* ——— Player rendering ——— */

static void draw_player(const Game *game) {
    const Player *player = &game->player;
    Vector2 pos = actor_pixel_position(&player->actor);
    Color tint = WHITE;

    if (player->state == PSTATE_DEAD) {
        float alpha = player->death_timer / 0.6f;
        if (alpha < 0.0f) alpha = 0.0f;
        tint.a = (unsigned char)(255.0f * alpha);
    } else if (player->state == PSTATE_DIG) {
        tint = (Color){ 180, 230, 255, 255 };
    } else if (player->state == PSTATE_FALL) {
        tint = (Color){ 200, 220, 255, 255 };
    }

    draw_sprite_flipped(game->sprites[TEX_PLAYER], pos.x, pos.y,
                        player->actor.facing, tint);
}

/* ——— Guard rendering ——— */

static void draw_guards(const Game *game) {
    for (int i = 0; i < game->world.guard_count; i++) {
        const Guard *guard = &game->guards[i];
        if (!guard->active || guard->state == GSTATE_RESPAWN) continue;

        Vector2 pos = actor_pixel_position(&guard->actor);
        Color tint = WHITE;
        if (guard->state == GSTATE_TRAPPED)
            tint = (Color){ 180, 160, 150, 255 };
        else if (guard->state == GSTATE_FALL)
            tint = (Color){ 255, 220, 210, 255 };

        draw_sprite_flipped(game->sprites[TEX_GUARD], pos.x, pos.y,
                            guard->actor.facing, tint);

        if (guard->carries_gold) {
            DrawCircle((int)pos.x, (int)pos.y - 18, 4.0f,
                       (Color){ 247, 196, 55, 255 });
        }
    }
}

/* ——— HUD ——— */

static void draw_hud(const Game *game) {
    DrawRectangleGradientV(0, 0, WINDOW_WIDTH, HUD_HEIGHT,
                           (Color){ 23, 32, 51, 255 }, (Color){ 6, 8, 13, 255 });
    DrawRectangle(0, HUD_HEIGHT - 2, WINDOW_WIDTH, 2, (Color){ 233, 177, 66, 255 });

    DrawText(TextFormat("SCORE %06d", game->score), 40, 38, 28,
             (Color){ 233, 221, 183, 255 });

    int drawn = game->lives < 9 ? game->lives : 9;
    int lx = (WINDOW_WIDTH - drawn * 22) / 2;
    for (int i = 0; i < drawn; i++) {
        DrawCircle(lx + i * 22 + 8, 48, 7.0f, SKYBLUE);
        DrawCircleLines(lx + i * 22 + 8, 48, 8.0f, RAYWHITE);
    }
    if (game->lives > 9) {
        DrawText(TextFormat("x%d", game->lives), lx + drawn * 22 + 4, 40, 18, RAYWHITE);
    }

    const char *lvl = TextFormat("LEVEL %02d / %02d", game->level_index + 1, MAX_LEVELS);
    int lvl_w = MeasureText(lvl, 28);
    DrawText(lvl, 1160 - lvl_w, 38, 28, (Color){ 233, 221, 183, 255 });

    DrawText(TextFormat("GOLD %02d / %02d",
             game->world.gold_remaining, game->world.gold_total),
             40, 76, 18, (Color){ 247, 196, 55, 200 });
    if (game->world.all_gold_collected) {
        DrawText("EXIT OPEN", 240, 76, 18, (Color){ 138, 235, 243, 200 });
    }
    if (!game->level_loaded_from_file) {
        DrawText("FALLBACK LEVEL", 400, 76, 14, (Color){ 248, 159, 99, 200 });
    }
}

/* ——— Overlays ——— */

static void draw_center_overlay(const char *title, const char *subtitle, Color tint) {
    DrawRectangle(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, (Color){ 0, 0, 0, 145 });
    int tw = MeasureText(title, 52);
    DrawText(title, (WINDOW_WIDTH - tw) / 2, 354, 52, tint);
    if (subtitle) {
        int sw = MeasureText(subtitle, 24);
        DrawText(subtitle, (WINDOW_WIDTH - sw) / 2, 424, 24, RAYWHITE);
    }
}

/* ——— Title screen ——— */

static void draw_title(const Game *game) {
    draw_parallax(game);
    DrawRectangle(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, (Color){ 0, 0, 0, 160 });

    const char *title = "LODE RUNNER";
    int title_w = MeasureText(title, 72);
    int title_x = (WINDOW_WIDTH - title_w) / 2;
    DrawText(title, title_x, 240, 72, (Color){ 247, 196, 55, 255 });
    DrawRectangle(title_x, 320, title_w, 3, (Color){ 233, 177, 66, 200 });

    const char *sub = "A Modernized Recreation";
    int sub_w = MeasureText(sub, 24);
    DrawText(sub, (WINDOW_WIDTH - sub_w) / 2, 345, 24, (Color){ 169, 184, 204, 180 });

    float pulse = 0.5f + 0.5f * sinf((float)GetTime() * 3.0f);
    unsigned char alpha = (unsigned char)(128.0f + 127.0f * pulse);
    const char *press = "PRESS ENTER TO START";
    int pw = MeasureText(press, 28);
    DrawText(press, (WINDOW_WIDTH - pw) / 2, 500, 28, (Color){ 255, 255, 255, alpha });

    const char *ctrl = "ARROWS/WASD/Gamepad: Move   Z/X/LB/RB: Dig   P/Start: Pause";
    int cw = MeasureText(ctrl, 16);
    DrawText(ctrl, (WINDOW_WIDTH - cw) / 2, 700, 16, (Color){ 120, 130, 150, 180 });

    const char *esc = "ESC/B to Quit";
    int ew = MeasureText(esc, 16);
    DrawText(esc, (WINDOW_WIDTH - ew) / 2, 730, 16, (Color){ 120, 130, 150, 140 });
}

/* ——— Main draw ——— */

void game_draw(Game *game) {
    BeginDrawing();
    ClearBackground((Color){ 8, 10, 14, 255 });

    if (game->screen == SCREEN_TITLE) {
        draw_title(game);
    } else {
        draw_parallax(game);

        float exit_alpha = 1.0f;
        if (game->world.all_gold_collected && game->exit_reveal_timer > 0.0f) {
            exit_alpha = 1.0f - game->exit_reveal_timer / EXIT_REVEAL_SEC;
            if (exit_alpha < 0.0f) exit_alpha = 0.0f;
        }

        bool shaking = game->shake_timer > 0.0f;
        if (shaking) {
            float intensity = (game->shake_timer / SHAKE_DURATION) * SHAKE_MAGNITUDE;
            Camera2D cam = { 0 };
            cam.zoom = 1.0f;
            cam.offset = (Vector2){
                (float)GetRandomValue(-100, 100) / 100.0f * intensity,
                (float)GetRandomValue(-100, 100) / 100.0f * intensity
            };
            BeginMode2D(cam);
        }

        draw_world(game, exit_alpha);
        draw_guards(game);
        draw_player(game);
        particles_draw(&game->particles);

        if (shaking) EndMode2D();

        draw_hud(game);

        if (game->screen == SCREEN_LEVEL_CLEAR) {
            if (game->victory) {
                draw_center_overlay("VICTORY",
                    "Press Enter for + NEW GAME +", (Color){ 247, 196, 55, 255 });
            } else {
                draw_center_overlay("LEVEL CLEAR",
                    "Next vault opening...", (Color){ 138, 235, 243, 255 });
            }
        } else if (game->screen == SCREEN_GAME_OVER) {
            draw_center_overlay("GAME OVER",
                "Press Enter to continue", (Color){ 226, 111, 98, 255 });
        } else if (game->paused) {
            draw_center_overlay("PAUSED",
                "P to resume / Esc for title", (Color){ 200, 200, 200, 255 });
        }
    }

    EndDrawing();
}

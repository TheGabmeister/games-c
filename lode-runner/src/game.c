#include "game.h"
#include "sounds.h"

static void load_level(Game *game, int level_index) {
    game->level_index = level_index;
    game->level_loaded_from_file = world_load_level(&game->world, game->level_index, game->level_status, sizeof(game->level_status));
}

static Color tile_base_color(TileID tile, bool exit_revealed) {
    switch (tile) {
        case TILE_BRICK: return (Color){ 133, 73, 45, 255 };
        case TILE_SOLID: return (Color){ 76, 82, 94, 255 };
        case TILE_LADDER: return (Color){ 202, 151, 78, 255 };
        case TILE_ROPE: return (Color){ 27, 31, 40, 255 };
        case TILE_GOLD: return (Color){ 31, 29, 28, 255 };
        case TILE_EXIT_LADDER:
            return exit_revealed ? (Color){ 93, 190, 207, 255 } : (Color){ 26, 29, 36, 255 };
        case TILE_TRAPDOOR: return (Color){ 102, 63, 47, 255 };
        case TILE_HOLE: return (Color){ 13, 14, 18, 255 };
        case TILE_EMPTY:
        default:
            return (Color){ 26, 29, 36, 255 };
    }
}

static Rectangle tile_rect(int r, int c) {
    return (Rectangle){
        (float)(PLAY_OFFSET_X + c * TILE_SIZE),
        (float)(PLAY_OFFSET_Y + r * TILE_SIZE),
        (float)TILE_SIZE,
        (float)TILE_SIZE
    };
}

static void draw_tile(TileID tile, int r, int c, bool exit_revealed) {
    Rectangle rect = tile_rect(r, c);
    DrawRectangleRec(rect, tile_base_color(tile, exit_revealed));
    DrawRectangleLinesEx(rect, 1.0f, (Color){ 45, 49, 60, 145 });

    if (tile == TILE_BRICK || tile == TILE_TRAPDOOR) {
        DrawLine((int)rect.x, (int)(rect.y + 12), (int)(rect.x + rect.width), (int)(rect.y + 12), (Color){ 94, 48, 33, 180 });
        DrawLine((int)rect.x, (int)(rect.y + 24), (int)(rect.x + rect.width), (int)(rect.y + 24), (Color){ 94, 48, 33, 180 });
        DrawLine((int)(rect.x + 18), (int)rect.y, (int)(rect.x + 18), (int)(rect.y + 12), (Color){ 94, 48, 33, 180 });
        DrawLine((int)(rect.x + 9), (int)(rect.y + 12), (int)(rect.x + 9), (int)(rect.y + 24), (Color){ 94, 48, 33, 180 });
    } else if (tile == TILE_SOLID) {
        DrawRectangleLinesEx((Rectangle){ rect.x + 4, rect.y + 4, rect.width - 8, rect.height - 8 }, 2.0f, (Color){ 110, 118, 130, 160 });
    } else if (tile == TILE_LADDER || (tile == TILE_EXIT_LADDER && exit_revealed)) {
        Color rail = tile == TILE_EXIT_LADDER ? (Color){ 138, 235, 243, 255 } : (Color){ 239, 198, 114, 255 };
        DrawLineEx((Vector2){ rect.x + 10, rect.y + 3 }, (Vector2){ rect.x + 10, rect.y + rect.height - 3 }, 3.0f, rail);
        DrawLineEx((Vector2){ rect.x + 26, rect.y + 3 }, (Vector2){ rect.x + 26, rect.y + rect.height - 3 }, 3.0f, rail);
        DrawLineEx((Vector2){ rect.x + 10, rect.y + 12 }, (Vector2){ rect.x + 26, rect.y + 12 }, 2.0f, rail);
        DrawLineEx((Vector2){ rect.x + 10, rect.y + 24 }, (Vector2){ rect.x + 26, rect.y + 24 }, 2.0f, rail);
    } else if (tile == TILE_ROPE) {
        DrawLineEx((Vector2){ rect.x, rect.y + 12 }, (Vector2){ rect.x + rect.width, rect.y + 12 }, 4.0f, (Color){ 195, 143, 74, 255 });
        DrawLineEx((Vector2){ rect.x + 8, rect.y + 14 }, (Vector2){ rect.x + 28, rect.y + 14 }, 1.0f, (Color){ 246, 201, 124, 180 });
    } else if (tile == TILE_GOLD) {
        Vector2 center = { rect.x + rect.width * 0.5f, rect.y + rect.height * 0.5f };
        DrawCircleV(center, 10.0f, (Color){ 247, 196, 55, 255 });
        DrawCircleV((Vector2){ center.x - 3, center.y - 3 }, 3.0f, (Color){ 255, 239, 146, 255 });
    } else if (tile == TILE_HOLE) {
        DrawRectangleLinesEx((Rectangle){ rect.x + 3, rect.y + 18, rect.width - 6, 12 }, 3.0f, (Color){ 77, 51, 40, 255 });
    }
}

static void draw_world(const World *world) {
    DrawRectangle(0, HUD_HEIGHT, WINDOW_WIDTH, WINDOW_HEIGHT - HUD_HEIGHT, (Color){ 15, 17, 23, 255 });

    for (int r = 0; r < GRID_ROWS; r++) {
        for (int c = 0; c < GRID_COLS; c++) {
            draw_tile(world->tiles[r][c], r, c, world->all_gold_collected);
        }
    }

    Rectangle player_rect = tile_rect(world->player_spawn_r, world->player_spawn_c);
    DrawCircle((int)(player_rect.x + player_rect.width * 0.5f), (int)(player_rect.y + player_rect.height * 0.5f), 11.0f, SKYBLUE);
    DrawCircleLines((int)(player_rect.x + player_rect.width * 0.5f), (int)(player_rect.y + player_rect.height * 0.5f), 12.0f, RAYWHITE);

    for (int i = 0; i < world->guard_spawn_count; i++) {
        Rectangle guard_rect = tile_rect(world->guard_spawn_r[i], world->guard_spawn_c[i]);
        DrawRectangleRounded((Rectangle){ guard_rect.x + 9, guard_rect.y + 8, 18, 20 }, 0.2f, 4, (Color){ 219, 72, 68, 255 });
        DrawRectangleLinesEx((Rectangle){ guard_rect.x + 9, guard_rect.y + 8, 18, 20 }, 1.0f, RAYWHITE);
    }
}

static void draw_hud(const Game *game) {
    DrawRectangle(0, 0, WINDOW_WIDTH, HUD_HEIGHT, (Color){ 10, 14, 24, 255 });
    DrawRectangleGradientV(0, 0, WINDOW_WIDTH, HUD_HEIGHT, (Color){ 23, 32, 51, 255 }, (Color){ 6, 8, 13, 255 });
    DrawRectangle(0, HUD_HEIGHT - 2, WINDOW_WIDTH, 2, (Color){ 233, 177, 66, 255 });

    DrawText("LODE RUNNER", 40, 30, 32, RAYWHITE);
    DrawText(TextFormat("LEVEL %02d / %02d", game->level_index + 1, MAX_LEVELS), 960, 34, 24, (Color){ 233, 221, 183, 255 });
    DrawText(TextFormat("GOLD %02d / %02d", game->world.gold_remaining, game->world.gold_total), 420, 38, 22, (Color){ 247, 196, 55, 255 });
    DrawText(TextFormat("GUARDS %d", game->world.guard_count), 640, 38, 22, (Color){ 226, 111, 98, 255 });

    if (!game->level_loaded_from_file) {
        DrawText("FALLBACK LEVEL", 40, 72, 16, (Color){ 248, 159, 99, 255 });
    }
}

void game_init(Game *game) {
    memset(game, 0, sizeof(*game));
    load_level(game, 0);
}

void game_update(Game *game) {
    if (IsKeyPressed(KEY_PAGE_DOWN)) {
        load_level(game, (game->level_index + 1) % MAX_LEVELS);
    }

    if (IsKeyPressed(KEY_PAGE_UP)) {
        int next = game->level_index - 1;
        if (next < 0) next = MAX_LEVELS - 1;
        load_level(game, next);
    }
}

void game_draw(Game *game) {
    BeginDrawing();
    ClearBackground((Color){ 8, 10, 14, 255 });

    draw_hud(game);
    draw_world(&game->world);

    EndDrawing();
}

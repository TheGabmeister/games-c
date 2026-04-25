#include "game.h"
#include "sounds.h"

static void load_level(Game *game, int level_index) {
    game->level_index = level_index;
    game->level_loaded_from_file = world_load_level(&game->world, game->level_index, game->level_status, sizeof(game->level_status));
    player_spawn(&game->player, &game->world);
    guards_spawn_from_world(game->guards, &game->world);
    world_rebuild_pursuit(&game->world, game->player.actor.tile_r, game->player.actor.tile_c, game->pursuit);
}

static void start_new_run(Game *game) {
    game->score = 0;
    game->lives = PLAYER_LIVES;
    game->screen = SCREEN_PLAY;
    game->screen_timer = 0.0f;
    game->victory = false;
    load_level(game, 0);
}

static void start_level_clear(Game *game) {
    game->score += SCORE_LEVEL_CLEAR;
    game->lives++;
    game->screen = SCREEN_LEVEL_CLEAR;
    game->screen_timer = 1.5f;
    game->victory = game->level_index >= MAX_LEVELS - 1;
    sound_play(game, SOUND_LEVEL_CLEAR);
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

static void draw_tile(TileID tile, int r, int c, bool exit_revealed, float hole_timer) {
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
        Color rim = (Color){ 77, 51, 40, 255 };
        if (hole_timer <= DIG_REFILL_SEC - DIG_WARN_FLASH_AT) {
            rim = RED;
        } else if (hole_timer <= DIG_REFILL_SEC - DIG_WARN_PULSE_AT) {
            rim = ORANGE;
        }
        DrawRectangleLinesEx((Rectangle){ rect.x + 3, rect.y + 18, rect.width - 6, 12 }, 3.0f, rim);
    }
}

static void draw_world(const World *world) {
    DrawRectangle(0, HUD_HEIGHT, WINDOW_WIDTH, WINDOW_HEIGHT - HUD_HEIGHT, (Color){ 15, 17, 23, 255 });

    for (int r = 0; r < GRID_ROWS; r++) {
        for (int c = 0; c < GRID_COLS; c++) {
            draw_tile(world->tiles[r][c], r, c, world->all_gold_collected, world->hole_timer[r][c]);
        }
    }
}

static void draw_player(const Player *player) {
    Vector2 pos = player_pixel_position(player);
    Color body = SKYBLUE;

    if (player->state == PSTATE_DEAD) {
        float alpha = player->death_timer / 0.6f;
        if (alpha < 0.0f) alpha = 0.0f;
        body.a = (unsigned char)(255.0f * alpha);
    } else if (player->state == PSTATE_DIG) {
        body = (Color){ 80, 210, 255, 255 };
    } else if (player->state == PSTATE_FALL) {
        body = (Color){ 111, 176, 255, 255 };
    }

    DrawCircleV(pos, 12.0f, body);
    DrawCircleLines((int)pos.x, (int)pos.y, 13.0f, RAYWHITE);
    DrawRectangle((int)pos.x - 5, (int)pos.y - 3, 10, 11, (Color){ 15, 38, 68, body.a });
    DrawCircle((int)pos.x + (player->actor.facing == DIR_RIGHT ? 4 : -4), (int)pos.y - 4, 2.0f, RAYWHITE);
}

static void draw_guards(const Game *game) {
    for (int i = 0; i < game->world.guard_count; i++) {
        const Guard *guard = &game->guards[i];
        if (!guard->active || guard->state == GSTATE_RESPAWN) {
            continue;
        }

        Vector2 pos = guard_pixel_position(guard);
        Color body = (Color){ 219, 72, 68, 255 };
        if (guard->state == GSTATE_TRAPPED) {
            body = (Color){ 168, 84, 68, 255 };
        } else if (guard->state == GSTATE_FALL) {
            body = (Color){ 238, 104, 86, 255 };
        }

        DrawRectangleRounded((Rectangle){ pos.x - 10, pos.y - 12, 20, 24 }, 0.25f, 4, body);
        DrawRectangleLinesEx((Rectangle){ pos.x - 10, pos.y - 12, 20, 24 }, 1.0f, RAYWHITE);
        DrawCircle((int)pos.x + (guard->actor.facing == DIR_RIGHT ? 4 : -4), (int)pos.y - 5, 2.0f, RAYWHITE);
        if (guard->carries_gold) {
            DrawCircle((int)pos.x, (int)pos.y - 18, 4.0f, (Color){ 247, 196, 55, 255 });
        }
    }
}

static void draw_center_overlay(const char *title, const char *subtitle, Color tint) {
    DrawRectangle(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, (Color){ 0, 0, 0, 145 });
    int title_width = MeasureText(title, 52);
    DrawText(title, (WINDOW_WIDTH - title_width) / 2, 354, 52, tint);
    if (subtitle != NULL) {
        int subtitle_width = MeasureText(subtitle, 24);
        DrawText(subtitle, (WINDOW_WIDTH - subtitle_width) / 2, 424, 24, RAYWHITE);
    }
}

static void draw_hud(const Game *game) {
    DrawRectangle(0, 0, WINDOW_WIDTH, HUD_HEIGHT, (Color){ 10, 14, 24, 255 });
    DrawRectangleGradientV(0, 0, WINDOW_WIDTH, HUD_HEIGHT, (Color){ 23, 32, 51, 255 }, (Color){ 6, 8, 13, 255 });
    DrawRectangle(0, HUD_HEIGHT - 2, WINDOW_WIDTH, 2, (Color){ 233, 177, 66, 255 });

    DrawText("LODE RUNNER", 40, 30, 32, RAYWHITE);
    DrawText(TextFormat("SCORE %06d", game->score), 258, 38, 22, (Color){ 233, 221, 183, 255 });
    DrawText(TextFormat("LEVEL %02d / %02d", game->level_index + 1, MAX_LEVELS), 960, 34, 24, (Color){ 233, 221, 183, 255 });
    DrawText(TextFormat("GOLD %02d / %02d", game->world.gold_remaining, game->world.gold_total), 420, 38, 22, (Color){ 247, 196, 55, 255 });
    DrawText(TextFormat("GUARDS %d", game->world.guard_count), 640, 38, 22, (Color){ 226, 111, 98, 255 });
    DrawText(TextFormat("LIVES %d", game->lives), 780, 38, 22, (Color){ 156, 212, 255, 255 });
    DrawText(player_state_name(game->player.state), 40, 72, 16, (Color){ 169, 184, 204, 255 });
    if (game->world.all_gold_collected) {
        DrawText("EXIT OPEN", 150, 72, 16, (Color){ 138, 235, 243, 255 });
    }

    if (!game->level_loaded_from_file) {
        DrawText("FALLBACK LEVEL", 250, 72, 16, (Color){ 248, 159, 99, 255 });
    }
}

void game_init(Game *game) {
    memset(game, 0, sizeof(*game));
    start_new_run(game);
}

static void game_update_play(Game *game, float dt) {
    bool rebuild_pursuit = false;

    if (IsKeyPressed(KEY_PAGE_DOWN)) {
        load_level(game, (game->level_index + 1) % MAX_LEVELS);
    }

    if (IsKeyPressed(KEY_PAGE_UP)) {
        int next = game->level_index - 1;
        if (next < 0) next = MAX_LEVELS - 1;
        load_level(game, next);
    }

    WorldTickResult wr = world_update_holes(&game->world, dt, game->player.actor.tile_r, game->player.actor.tile_c);
    if (wr.refilled_hole) {
        sound_play(game, SOUND_REFILL);
        rebuild_pursuit = true;

        for (int i = 0; i < game->world.guard_count; i++) {
            Guard *guard = &game->guards[i];
            if (guard->active && guard->state != GSTATE_RESPAWN &&
                wr.refilled_tiles[guard->actor.tile_r][guard->actor.tile_c]) {
                bool dropped_gold = guard_kill_in_refill(guard, &game->world);
                (void)dropped_gold;
                game->score += SCORE_GUARD_KILL;
                sound_play(game, SOUND_GUARD_DIE);
            }
        }
    }
    if (wr.trapped_target) {
        player_start_death(&game->player);
        sound_play(game, SOUND_PLAYER_DIE);
    }

    PlayerTickResult pr = player_update(&game->player, &game->world, dt);
    if (pr.collected_gold) {
        game->score += SCORE_GOLD;
        sound_play(game, SOUND_COIN);
        if (game->world.all_gold_collected) {
            sound_play(game, SOUND_GOLD_COMPLETE);
        }
    }
    if (pr.dug_brick) {
        rebuild_pursuit = true;
        sound_play(game, SOUND_DIG);
    }
    if (pr.committed_new_tile) {
        rebuild_pursuit = true;
    }

    if (pr.reached_exit) {
        start_level_clear(game);
        return;
    }

    if (rebuild_pursuit) {
        world_rebuild_pursuit(&game->world, game->player.actor.tile_r, game->player.actor.tile_c, game->pursuit);
    }

    for (int i = 0; i < game->world.guard_count; i++) {
        GuardTickResult gr = guard_update(&game->guards[i], i, game->guards, &game->world, &game->player, game->pursuit, dt);
        if (gr.fell_in_hole) {
            sound_play(game, SOUND_GUARD_FALL);
        }
    }

    if (game->player.state != PSTATE_DEAD) {
        for (int i = 0; i < game->world.guard_count; i++) {
            if (guard_can_catch_player(&game->guards[i], &game->player)) {
                player_start_death(&game->player);
                sound_play(game, SOUND_PLAYER_DIE);
                break;
            }
        }
    }

    if (pr.died) {
        game->lives--;
        if (game->lives <= 0) {
            game->screen = SCREEN_GAME_OVER;
            game->screen_timer = 0.0f;
            return;
        }
        load_level(game, game->level_index);
    }
}

void game_update(Game *game) {
    float dt = GetFrameTime();

    if (game->screen == SCREEN_LEVEL_CLEAR) {
        if (game->victory) {
            if (IsKeyPressed(KEY_ENTER)) {
                game->screen = SCREEN_PLAY;
                game->victory = false;
                load_level(game, 0);
            }
            return;
        }

        game->screen_timer -= dt;
        if (game->screen_timer <= 0.0f || IsKeyPressed(KEY_ENTER)) {
            game->screen = SCREEN_PLAY;
            load_level(game, (game->level_index + 1) % MAX_LEVELS);
        }
        return;
    }

    if (game->screen == SCREEN_GAME_OVER) {
        if (IsKeyPressed(KEY_ENTER)) {
            start_new_run(game);
        }
        return;
    }

    game_update_play(game, dt);
}

void game_draw(Game *game) {
    BeginDrawing();
    ClearBackground((Color){ 8, 10, 14, 255 });

    draw_hud(game);
    draw_world(&game->world);
    draw_guards(game);
    draw_player(&game->player);

    if (game->screen == SCREEN_LEVEL_CLEAR) {
        if (game->victory) {
            draw_center_overlay("VICTORY", "Press Enter for + NEW GAME +", (Color){ 247, 196, 55, 255 });
        } else {
            draw_center_overlay("LEVEL CLEAR", "Next vault opening...", (Color){ 138, 235, 243, 255 });
        }
    } else if (game->screen == SCREEN_GAME_OVER) {
        draw_center_overlay("GAME OVER", "Press Enter to restart", (Color){ 226, 111, 98, 255 });
    }

    EndDrawing();
}

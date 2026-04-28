#include "game.h"
#include "sounds.h"
#include "hud.h"
#include "textures.h"
#include <stdio.h>

static void load_screen_at(Game *game, int sx, int sy) {
    char path[SCREEN_PATH_MAX];
    snprintf(path, sizeof(path), "assets/screens/%02d_%02d.txt", sx, sy);
    if (!screen_load(&game->current_screen, path)) {
        memset(&game->current_screen, TILE_FLOOR, sizeof(game->current_screen));
    }
    game->screen_x = sx;
    game->screen_y = sy;
}

static void load_cave_screen(Game *game, const char *cave_name) {
    char path[SCREEN_PATH_MAX];
    snprintf(path, sizeof(path), "assets/caves/%s.txt", cave_name);
    if (!screen_load(&game->current_screen, path)) {
        memset(&game->current_screen, TILE_FLOOR, sizeof(game->current_screen));
    }
}

static bool can_transition(int screen_x, int screen_y, Direction dir) {
    int nx = screen_x, ny = screen_y;
    switch (dir) {
        case DIR_N: ny--; break;
        case DIR_S: ny++; break;
        case DIR_W: nx--; break;
        case DIR_E: nx++; break;
        default: return false;
    }
    if (nx < 0 || nx >= OVERWORLD_COLS || ny < 0 || ny >= OVERWORLD_ROWS)
        return false;
    return screen_file_exists(nx, ny);
}

static void start_scroll_transition(Game *game, Direction dir) {
    int nx = game->screen_x, ny = game->screen_y;
    switch (dir) {
        case DIR_N: ny--; break;
        case DIR_S: ny++; break;
        case DIR_W: nx--; break;
        case DIR_E: nx++; break;
        default: return;
    }

    char path[SCREEN_PATH_MAX];
    snprintf(path, sizeof(path), "assets/screens/%02d_%02d.txt", nx, ny);
    if (!screen_load(&game->next_screen, path)) return;

    game->trans_player_start = game->player.pos;

    Vector2 end = game->player.pos;
    switch (dir) {
        case DIR_N: end.y = PLAY_AREA_Y + (SCREEN_TILES_Y - 1) * TILE_SIZE; break;
        case DIR_S: end.y = PLAY_AREA_Y; break;
        case DIR_W: end.x = (SCREEN_TILES_X - 1) * TILE_SIZE; break;
        case DIR_E: end.x = 0; break;
        default: break;
    }
    game->trans_player_end = end;
    game->warp_dest_x = nx;
    game->warp_dest_y = ny;

    game->trans_type = TRANS_SCROLL;
    camera_start_scroll(&game->cam, dir);
    game->state = STATE_TRANSITION;
}

static void check_edge_transition(Game *game) {
    if (game->in_cave) return;

    Player *p = &game->player;
    Direction dir = (Direction)-1;

    if (p->pos.y <= PLAY_AREA_Y && p->facing == DIR_N)
        dir = DIR_N;
    else if (p->pos.y >= PLAY_AREA_Y + (SCREEN_TILES_Y - 1) * TILE_SIZE && p->facing == DIR_S)
        dir = DIR_S;
    else if (p->pos.x <= 0 && p->facing == DIR_W)
        dir = DIR_W;
    else if (p->pos.x >= (SCREEN_TILES_X - 1) * TILE_SIZE && p->facing == DIR_E)
        dir = DIR_E;

    if ((int)dir == -1) return;
    if (!can_transition(game->screen_x, game->screen_y, dir)) return;

    start_scroll_transition(game, dir);
}

static void check_warp(Game *game) {
    int col = (int)(game->player.pos.x / TILE_SIZE);
    int row = (int)((game->player.pos.y - PLAY_AREA_Y) / TILE_SIZE);

    if (col < 0 || col >= SCREEN_TILES_X || row < 0 || row >= SCREEN_TILES_Y) return;

    TileType t = (TileType)game->current_screen.tiles[row][col];
    if (t != TILE_STAIRS) return;

    const Warp *w = screen_warp_at(&game->current_screen, col, row);
    if (!w) return;

    if (strcmp(w->dest, "return") == 0) {
        game->warp_dest_x = game->return_screen_x;
        game->warp_dest_y = game->return_screen_y;
        game->warp_dest_name[0] = '\0';
    } else if (strncmp(w->dest, "cave_", 5) == 0) {
        game->return_screen_x = game->screen_x;
        game->return_screen_y = game->screen_y;
        game->return_tile_col = col;
        game->return_tile_row = row;
        strncpy(game->warp_dest_name, w->dest, WARP_DEST_MAX - 1);
        game->warp_dest_name[WARP_DEST_MAX - 1] = '\0';
    } else {
        int dx, dy;
        if (sscanf(w->dest, "%d_%d", &dx, &dy) != 2) return;
        game->warp_dest_x = dx;
        game->warp_dest_y = dy;
        game->warp_dest_name[0] = '\0';
    }

    game->trans_type = TRANS_FADE;
    camera_start_fade(&game->cam);
    game->state = STATE_TRANSITION;
}

void game_init(Game *game) {
    memset(game, 0, sizeof(*game));
    game->state = STATE_PLAY;

    player_init(&game->player);
    load_screen_at(game, START_SCREEN_X, START_SCREEN_Y);
}

void game_update(Game *game) {
    float dt = GetFrameTime();

    if (game->music_loaded) {
        UpdateMusicStream(game->overworld_music);
    }

    switch (game->state) {
        case STATE_PLAY:
            player_update(&game->player, &game->current_screen, dt);
            check_warp(game);
            if (game->state == STATE_PLAY) {
                check_edge_transition(game);
            }
            break;

        case STATE_TRANSITION:
            camera_update(&game->cam);

            if (camera_at_midpoint(&game->cam)) {
                if (game->warp_dest_name[0] != '\0') {
                    // Entering a cave
                    load_cave_screen(game, game->warp_dest_name);
                    game->in_cave = true;
                    // Land on the cave's return warp tile (or center as fallback)
                    const Warp *rw = NULL;
                    for (int i = 0; i < game->current_screen.warp_count; i++) {
                        if (game->current_screen.warps[i].active &&
                            strcmp(game->current_screen.warps[i].dest, "return") == 0) {
                            rw = &game->current_screen.warps[i];
                            break;
                        }
                    }
                    if (rw) {
                        game->player.pos.x = rw->tile_col * TILE_SIZE;
                        game->player.pos.y = PLAY_AREA_Y + (rw->tile_row - 1) * TILE_SIZE;
                    } else {
                        game->player.pos.x = 7 * TILE_SIZE;
                        game->player.pos.y = PLAY_AREA_Y + 5 * TILE_SIZE;
                    }
                } else if (game->in_cave) {
                    // Returning from cave to overworld
                    load_screen_at(game, game->warp_dest_x, game->warp_dest_y);
                    game->in_cave = false;
                    game->player.pos.x = game->return_tile_col * TILE_SIZE;
                    game->player.pos.y = PLAY_AREA_Y + (game->return_tile_row - 1) * TILE_SIZE;
                } else {
                    // Overworld-to-overworld warp
                    load_screen_at(game, game->warp_dest_x, game->warp_dest_y);
                    game->player.pos.x = 7 * TILE_SIZE;
                    game->player.pos.y = PLAY_AREA_Y + 5 * TILE_SIZE;
                }
            }

            if (!camera_is_active(&game->cam)) {
                if (game->trans_type == TRANS_SCROLL) {
                    game->current_screen = game->next_screen;
                    game->screen_x = game->warp_dest_x;
                    game->screen_y = game->warp_dest_y;
                    game->player.pos = game->trans_player_end;
                }
                game->state = STATE_PLAY;
            }
            break;

        default:
            break;
    }
}

void game_draw(Game *game) {
    BeginDrawing();
    ClearBackground((Color){ 20, 24, 32, 255 });

    if (game->state == STATE_TRANSITION && game->cam.type == TRANS_SCROLL) {
        int ox_old, oy_old, ox_new, oy_new;
        camera_scroll_offset_old(&game->cam, &ox_old, &oy_old);
        camera_scroll_offset_new(&game->cam, &ox_new, &oy_new);

        BeginScissorMode(0, PLAY_AREA_Y, WINDOW_WIDTH, PLAY_AREA_HEIGHT);
        screen_draw_offset(&game->current_screen, ox_old, oy_old);
        screen_draw_offset(&game->next_screen, ox_new, oy_new);

        float t = (float)game->cam.timer / game->cam.total_frames;
        Vector2 player_pos = {
            game->trans_player_start.x + (game->trans_player_end.x - game->trans_player_start.x) * t,
            game->trans_player_start.y + (game->trans_player_end.y - game->trans_player_start.y) * t,
        };
        Vector2 old_pos = game->player.pos;
        game->player.pos = player_pos;
        player_draw(&game->player);
        game->player.pos = old_pos;

        EndScissorMode();
    } else if (game->state == STATE_TRANSITION && game->cam.type == TRANS_FADE) {
        screen_draw(&game->current_screen);
        player_draw(&game->player);
        unsigned char alpha = camera_fade_alpha(&game->cam);
        DrawRectangle(0, PLAY_AREA_Y, WINDOW_WIDTH, PLAY_AREA_HEIGHT,
                      (Color){ 0, 0, 0, alpha });
    } else {
        screen_draw(&game->current_screen);
        player_draw(&game->player);
    }

    hud_draw(&game->player, game->screen_x, game->screen_y);

    EndDrawing();
}

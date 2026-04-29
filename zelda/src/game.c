#include "game.h"
#include "combat.h"
#include "navigation.h"
#include "dungeon_interact.h"
#include "sounds.h"
#include "hud.h"
#include "textures.h"
#include "debug.h"
#include "input.h"
#include "vfx.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

void game_init(Game *game) {
    memset(game, 0, sizeof(*game));
    game->state = STATE_PLAY;

    player_init(&game->player);
    nav_load_screen(game, START_SCREEN_X, START_SCREEN_Y);
}

void game_update(Game *game) {
    if (IsKeyPressed(KEY_F3)) debug_toggle();

    float dt = GetFrameTime();

    if (game->in_dungeon) {
        if (game->boss_music_loaded && game->current_screen.is_boss_room &&
            !game->dungeon.boss_defeated) {
            UpdateMusicStream(game->boss_music);
        } else if (game->dungeon_music_loaded) {
            UpdateMusicStream(game->dungeon_music);
        }
    } else if (game->music_loaded) {
        UpdateMusicStream(game->overworld_music);
    }

    switch (game->state) {
        case STATE_PLAY: {
            if (input_pause()) {
                game->state = STATE_PAUSE;
                break;
            }
            PlayerState prev_state = game->player.state;
            player_update(&game->player, &game->current_screen,
                          game->projectiles, &game->projectile_count, dt);
            if (game->player.state == PSTATE_ATTACKING && prev_state != PSTATE_ATTACKING) {
                sound_play(game, SOUND_SWORD_SWING);
            }
            if (game->player.state == PSTATE_USING_ITEM && prev_state != PSTATE_USING_ITEM &&
                game->player.inventory.equipped == ITEM_CANDLE &&
                game->in_dungeon && game->current_screen.is_dark) {
                uint64_t rbit = dungeon_room_bit(game->dungeon.room_x, game->dungeon.room_y);
                game->dungeon.rooms_lit |= rbit;
                sound_play(game, SOUND_SECRET);
            }
            enemies_update(game->enemies, game->enemy_count,
                           game->player.pos, &game->current_screen,
                           game->projectiles, &game->projectile_count, dt);
            projectiles_update(game->projectiles, &game->projectile_count,
                               &game->current_screen, game->player.pos, dt);
            combat_check_bombs(game);
            vfx_update();
            combat_check(game);
            pickups_update(game->pickups, game->pickup_count);
            combat_check_pickups(game);
            dungeon_check_locked_door(game);
            dungeon_check_shutter_room(game);
            dungeon_check_push_block(game);
            dungeon_check_items(game);
            if (game->state != STATE_PLAY) break;
            if (game->player.health <= 0) {
                game->state = STATE_DEATH;
                game->death_timer = 90;
                break;
            }
            if (game->player.health > 0 &&
                game->player.health <= LOW_HEALTH_THRESHOLD) {
                game->low_health_counter++;
                if (game->low_health_counter >= LOW_HEALTH_BEEP_FRAMES) {
                    sound_play(game, SOUND_LOW_HEALTH);
                    game->low_health_counter = 0;
                }
            } else {
                game->low_health_counter = 0;
            }
            nav_check_warp(game);
            if (game->state == STATE_PLAY) {
                nav_check_edge_transition(game);
            }
            break;
        }

        case STATE_TRANSITION:
            camera_update(&game->cam);

            if (camera_at_midpoint(&game->cam)) {
                if (game->warp_dest_name[0] != '\0' &&
                    strncmp(game->warp_dest_name, "droom_", 6) == 0 && game->in_dungeon) {
                    int drx = game->warp_dest_x, dry = game->warp_dest_y;
                    bool was_boss = game->current_screen.is_boss_room;
                    nav_load_dungeon_room(game, drx, dry);
                    bool is_boss = game->current_screen.is_boss_room && !game->dungeon.boss_defeated;
                    if (is_boss && !was_boss) {
                        if (game->dungeon_music_loaded) StopMusicStream(game->dungeon_music);
                        if (game->boss_music_loaded) PlayMusicStream(game->boss_music);
                        sound_play(game, SOUND_BOSS_ROAR);
                    } else if (!is_boss && was_boss) {
                        if (game->boss_music_loaded) StopMusicStream(game->boss_music);
                        if (game->dungeon_music_loaded) PlayMusicStream(game->dungeon_music);
                    }
                    game->player.pos.x = 7.0f * TILE_SIZE;
                    game->player.pos.y = PLAY_AREA_Y + 5.0f * TILE_SIZE;
                    game->warp_dest_name[0] = '\0';
                } else if (game->warp_dest_name[0] != '\0' &&
                    strncmp(game->warp_dest_name, "dungeon_", 8) == 0) {
                    int dungeon_id = 0;
                    sscanf(game->warp_dest_name + 8, "%d", &dungeon_id);
                    memset(&game->dungeon, 0, sizeof(game->dungeon));
                    game->dungeon.id = dungeon_id;
                    game->dungeon.entrance_room_x = 0;
                    game->dungeon.entrance_room_y = 0;
                    dungeon_scan_rooms(&game->dungeon);
                    game->in_dungeon = true;

                    if (game->music_loaded) {
                        StopMusicStream(game->overworld_music);
                    }
                    char dmus_path[64];
                    snprintf(dmus_path, sizeof(dmus_path), "assets/music/dungeon%d.ogg", dungeon_id);
                    if (FileExists(dmus_path)) {
                        game->dungeon_music = LoadMusicStream(dmus_path);
                        game->dungeon_music_loaded = IsMusicValid(game->dungeon_music);
                        if (game->dungeon_music_loaded) PlayMusicStream(game->dungeon_music);
                    }
                    snprintf(dmus_path, sizeof(dmus_path), "assets/music/boss%d.ogg", dungeon_id);
                    if (FileExists(dmus_path)) {
                        game->boss_music = LoadMusicStream(dmus_path);
                        game->boss_music_loaded = IsMusicValid(game->boss_music);
                    }

                    nav_load_dungeon_room(game, 0, 0);
                    nav_position_at_return_warp(game);
                    game->warp_dest_name[0] = '\0';
                } else if (game->warp_dest_name[0] != '\0') {
                    nav_load_cave(game, game->warp_dest_name);
                    game->in_cave = true;
                    nav_position_at_return_warp(game);
                } else if (game->in_dungeon) {
                    nav_load_screen(game, game->warp_dest_x, game->warp_dest_y);
                    game->in_dungeon = false;
                    if (game->dungeon_music_loaded) {
                        StopMusicStream(game->dungeon_music);
                        UnloadMusicStream(game->dungeon_music);
                        game->dungeon_music_loaded = false;
                    }
                    if (game->boss_music_loaded) {
                        StopMusicStream(game->boss_music);
                        UnloadMusicStream(game->boss_music);
                        game->boss_music_loaded = false;
                    }
                    if (game->music_loaded) {
                        PlayMusicStream(game->overworld_music);
                    }
                    game->player.pos.x = (float)(game->dungeon_return_tile_col * TILE_SIZE);
                    game->player.pos.y = (float)(PLAY_AREA_Y + (game->dungeon_return_tile_row - 1) * TILE_SIZE);
                } else if (game->in_cave) {
                    nav_load_screen(game, game->warp_dest_x, game->warp_dest_y);
                    game->in_cave = false;
                    game->player.pos.x = (float)(game->return_tile_col * TILE_SIZE);
                    game->player.pos.y = (float)(PLAY_AREA_Y + (game->return_tile_row - 1) * TILE_SIZE);
                } else {
                    nav_load_screen(game, game->warp_dest_x, game->warp_dest_y);
                    game->player.pos.x = 7.0f * TILE_SIZE;
                    game->player.pos.y = PLAY_AREA_Y + 5.0f * TILE_SIZE;
                }
            }

            if (!camera_is_active(&game->cam)) {
                if (game->trans_type == TRANS_SCROLL) {
                    game->player.pos = game->trans_player_end;
                    if (game->in_dungeon) {
                        bool was_boss = game->current_screen.is_boss_room;
                        nav_load_dungeon_room(game, game->warp_dest_x, game->warp_dest_y);
                        bool is_boss = game->current_screen.is_boss_room && !game->dungeon.boss_defeated;
                        if (is_boss && !was_boss) {
                            if (game->dungeon_music_loaded) StopMusicStream(game->dungeon_music);
                            if (game->boss_music_loaded) PlayMusicStream(game->boss_music);
                            sound_play(game, SOUND_BOSS_ROAR);
                        } else if (!is_boss && was_boss) {
                            if (game->boss_music_loaded) StopMusicStream(game->boss_music);
                            if (game->dungeon_music_loaded) PlayMusicStream(game->dungeon_music);
                        }
                        Rectangle ph = player_hitbox(&game->player);
                        if (screen_tile_blocked(&game->current_screen, ph)) {
                            if (game->player.pos.x <= 0)
                                game->player.pos.x = (float)TILE_SIZE;
                            else if (game->player.pos.x >= (SCREEN_TILES_X - 1) * TILE_SIZE)
                                game->player.pos.x = (float)((SCREEN_TILES_X - 2) * TILE_SIZE);
                            if (game->player.pos.y <= PLAY_AREA_Y)
                                game->player.pos.y = PLAY_AREA_Y + (float)TILE_SIZE;
                            else if (game->player.pos.y >= PLAY_AREA_Y + (SCREEN_TILES_Y - 1) * TILE_SIZE)
                                game->player.pos.y = PLAY_AREA_Y + (float)((SCREEN_TILES_Y - 2) * TILE_SIZE);
                        }
                    } else {
                        game->current_screen = game->next_screen;
                        game->screen_x = game->warp_dest_x;
                        game->screen_y = game->warp_dest_y;
                        enemies_spawn(game->enemies, &game->enemy_count,
                                      &game->current_screen);
                        projectiles_clear(game->projectiles, &game->projectile_count);
                        pickups_clear(game->pickups, &game->pickup_count);
                        vfx_clear();
                    }
                }
                game->state = STATE_PLAY;
            }
            break;

        case STATE_DEATH:
            game->death_timer--;
            if (game->death_timer <= 0) {
                if (game->in_dungeon) {
                    game->player.health = 6;
                    game->player.state = PSTATE_IDLE;
                    game->player.invuln_timer = 0;
                    game->player.knockback_timer = 0;
                    game->dungeon.rooms_cleared = 0;
                    game->dungeon.shutter_opened = 0;
                    game->dungeon.rooms_lit = 0;
                    nav_load_dungeon_room(game, game->dungeon.entrance_room_x,
                                      game->dungeon.entrance_room_y);
                    nav_position_at_return_warp(game);
                } else {
                    player_init(&game->player);
                    game->in_cave = false;
                    nav_load_screen(game, START_SCREEN_X, START_SCREEN_Y);
                }
                game->state = STATE_PLAY;
            }
            break;

        case STATE_ITEM_GET:
            game->item_get_timer--;
            if (game->item_get_timer <= 0) {
                game->state = STATE_PLAY;
            }
            break;

        case STATE_PAUSE:
            if (input_pause() || input_back()) {
                game->state = STATE_PLAY;
            } else {
                pause_screen_update(&game->pause_state, &game->player.inventory);
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
        enemies_draw(game->enemies, game->enemy_count);
        player_draw(&game->player);
        unsigned char alpha = camera_fade_alpha(&game->cam);
        DrawRectangle(0, PLAY_AREA_Y, WINDOW_WIDTH, PLAY_AREA_HEIGHT,
                      (Color){ 0, 0, 0, alpha });
    } else if (game->state == STATE_DEATH) {
        screen_draw(&game->current_screen);
        enemies_draw(game->enemies, game->enemy_count);
        player_draw(&game->player);
        int half = 45;
        if (game->death_timer < half) {
            unsigned char alpha = (unsigned char)(255 * (half - game->death_timer) / half);
            DrawRectangle(0, PLAY_AREA_Y, WINDOW_WIDTH, PLAY_AREA_HEIGHT,
                          (Color){ 0, 0, 0, alpha });
        }
    } else if (game->state == STATE_PAUSE) {
        screen_draw(&game->current_screen);
        enemies_draw(game->enemies, game->enemy_count);
        pickups_draw(game->pickups, game->pickup_count);
        projectiles_draw(game->projectiles, game->projectile_count);
        player_draw(&game->player);
        pause_screen_draw(&game->pause_state, game);
    } else if (game->state == STATE_ITEM_GET) {
        screen_draw(&game->current_screen);
        enemies_draw(game->enemies, game->enemy_count);
        player_draw(&game->player);
        DrawRectangle(0, PLAY_AREA_Y, WINDOW_WIDTH, PLAY_AREA_HEIGHT,
                      (Color){ 0, 0, 0, 160 });

        const char *item_name = "ITEM";
        Color item_color = GOLD;
        switch (game->item_get_type) {
            case ITEM_KEY:             item_name = "KEY";             item_color = YELLOW; break;
            case ITEM_MAP:             item_name = "DUNGEON MAP";     item_color = BLUE; break;
            case ITEM_COMPASS:         item_name = "COMPASS";         item_color = RED; break;
            case ITEM_HEART_CONTAINER: item_name = "HEART CONTAINER"; item_color = RED; break;
            case ITEM_FRAGMENT:        item_name = "RELIC FRAGMENT";  item_color = GOLD; break;
            case ITEM_BOOMERANG:       item_name = "BOOMERANG";       item_color = SKYBLUE; break;
            case ITEM_BOW:             item_name = "BOW";             item_color = BROWN; break;
            default: break;
        }

        int item_x = (int)game->player.pos.x + TILE_SIZE / 2 - 20;
        int item_y = (int)game->player.pos.y - TILE_SIZE;
        TextureID get_tex = dungeon_item_texture(game->item_get_type);
        assert(get_tex < TEX_COUNT && IsTextureValid(textures[get_tex]));
        DrawTexture(textures[get_tex], item_x - 12, item_y - 12, WHITE);

        int text_w = MeasureText(item_name, 30);
        DrawText(item_name, WINDOW_WIDTH / 2 - text_w / 2,
                 PLAY_AREA_Y + PLAY_AREA_HEIGHT / 2 + 60, 30, WHITE);
    } else {
        screen_draw(&game->current_screen);
        dungeon_draw_items(game);
        enemies_draw(game->enemies, game->enemy_count);
        pickups_draw(game->pickups, game->pickup_count);
        projectiles_draw(game->projectiles, game->projectile_count);
        player_draw(&game->player);
        vfx_draw();

        if (game->in_dungeon && game->current_screen.is_dark) {
            uint64_t rbit = dungeon_room_bit(game->dungeon.room_x, game->dungeon.room_y);
            if (!(game->dungeon.rooms_lit & rbit)) {
                DrawRectangle(0, PLAY_AREA_Y, WINDOW_WIDTH, PLAY_AREA_HEIGHT, BLACK);
            }
        }
    }

    hud_draw(game);
    debug_draw_game(game);
    debug_draw_overlay();

    EndDrawing();
}

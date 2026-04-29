#include "game.h"
#include "sounds.h"
#include "hud.h"
#include "textures.h"
#include "debug.h"
#include "input.h"
#include "vfx.h"
#include "raymath.h"
#include <stdio.h>
#include <math.h>
#include <stdlib.h>

static void load_screen_at(Game *game, int sx, int sy) {
    char path[SCREEN_PATH_MAX];
    snprintf(path, sizeof(path), "assets/screens/%02d_%02d.txt", sx, sy);
    if (!screen_load(&game->current_screen, path)) {
        memset(&game->current_screen, TILE_FLOOR, sizeof(game->current_screen));
    }
    game->screen_x = sx;
    game->screen_y = sy;
    enemies_spawn(game->enemies, &game->enemy_count, &game->current_screen);
    projectiles_clear(game->projectiles, &game->projectile_count);
    pickups_clear(game->pickups, &game->pickup_count);
    vfx_clear();
}

static void load_cave_screen(Game *game, const char *cave_name) {
    char path[SCREEN_PATH_MAX];
    snprintf(path, sizeof(path), "assets/caves/%s.txt", cave_name);
    if (!screen_load(&game->current_screen, path)) {
        memset(&game->current_screen, TILE_FLOOR, sizeof(game->current_screen));
    }
    enemies_spawn(game->enemies, &game->enemy_count, &game->current_screen);
    projectiles_clear(game->projectiles, &game->projectile_count);
    pickups_clear(game->pickups, &game->pickup_count);
    vfx_clear();
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

static void spawn_small_slimes(Game *game, Vector2 pos) {
    for (int s = 0; s < 2 && game->enemy_count < MAX_ENEMIES_PER_SCREEN; s++) {
        Enemy *e = &game->enemies[game->enemy_count++];
        *e = (Enemy){0};
        e->type = ENEMY_SLIME;
        e->subtype = 1;
        e->pos.x = pos.x + (s == 0 ? -16.0f : 16.0f);
        e->pos.y = pos.y;
        e->pos.x = Clamp(e->pos.x, 0, SCREEN_TILES_X * TILE_SIZE - TILE_SIZE);
        e->facing = DIR_S;
        e->state = ESTATE_IDLE;
        e->state_timer = 20 + rand() % 40;
        e->health = 1;
        e->active = true;
    }
}

static void try_spawn_drop(Game *game, Vector2 pos) {
    int roll = rand() % 100;
    if (roll < 35) return;
    PickupType type;
    if (roll < 55)      type = PICKUP_RUPEE;
    else if (roll < 70) type = PICKUP_HEART;
    else if (roll < 85) type = PICKUP_ARROW;
    else                type = PICKUP_BOMB;
    pickup_spawn(game->pickups, &game->pickup_count, type, pos);
}

static void check_pickups(Game *game) {
    Player *p = &game->player;
    Rectangle player_rect = player_hitbox(p);
    for (int i = 0; i < game->pickup_count; i++) {
        Pickup *pk = &game->pickups[i];
        if (!pk->active) continue;
        if (!CheckCollisionRecs(player_rect, pickup_hitbox(pk))) continue;

        switch (pk->type) {
            case PICKUP_RUPEE:
                p->inventory.rupees += pk->value;
                if (p->inventory.rupees > 255) p->inventory.rupees = 255;
                sound_play(game, SOUND_PICKUP_RUPEE);
                break;
            case PICKUP_HEART:
                p->health += pk->value;
                if (p->health > p->max_health) p->health = p->max_health;
                sound_play(game, SOUND_PICKUP_HEART);
                break;
            case PICKUP_BOMB:
                p->inventory.bombs += pk->value;
                if (p->inventory.bombs > p->inventory.bomb_capacity)
                    p->inventory.bombs = p->inventory.bomb_capacity;
                sound_play(game, SOUND_PICKUP_BOMB);
                break;
            case PICKUP_ARROW:
                p->inventory.arrows += pk->value;
                if (p->inventory.arrows > p->inventory.arrow_capacity)
                    p->inventory.arrows = p->inventory.arrow_capacity;
                sound_play(game, SOUND_PICKUP_BOMB);
                break;
            default: break;
        }
        pk->active = false;
    }
}

static void check_bomb_explosions(Game *game) {
    for (int pi = 0; pi < game->projectile_count; pi++) {
        Projectile *proj = &game->projectiles[pi];
        if (!proj->active || proj->type != PROJ_BOMB || proj->timer > 0) continue;

        sound_play(game, SOUND_BOMB_EXPLODE);
        float cx = proj->pos.x + TILE_SIZE / 2.0f;
        float cy = proj->pos.y + TILE_SIZE / 2.0f;
        float r2 = BOMB_BLAST_RADIUS * BOMB_BLAST_RADIUS;

        for (int i = 0; i < game->enemy_count; i++) {
            Enemy *e = &game->enemies[i];
            if (!e->active) continue;
            float dx = (e->pos.x + TILE_SIZE / 2.0f) - cx;
            float dy = (e->pos.y + TILE_SIZE / 2.0f) - cy;
            if (dx * dx + dy * dy <= r2) {
                enemy_take_damage(e, BOMB_DAMAGE);
                if (!e->active) {
                    sound_play(game, SOUND_ENEMY_DEATH);
                    try_spawn_drop(game, e->pos);
                }
            }
        }

        Player *p = &game->player;
        float pdx = (p->pos.x + TILE_SIZE / 2.0f) - cx;
        float pdy = (p->pos.y + TILE_SIZE / 2.0f) - cy;
        if (pdx * pdx + pdy * pdy <= r2) {
            Direction kb_dir;
            if (fabsf(pdx) > fabsf(pdy))
                kb_dir = (pdx > 0) ? DIR_E : DIR_W;
            else
                kb_dir = (pdy > 0) ? DIR_S : DIR_N;
            player_take_damage(p, BOMB_DAMAGE, kb_dir);
            sound_play(game, SOUND_PLAYER_DAMAGE);
        }

        int tc = (int)(cx / TILE_SIZE);
        int tr = (int)((cy - PLAY_AREA_Y) / TILE_SIZE);
        for (int dr = -2; dr <= 2; dr++) {
            for (int dc = -2; dc <= 2; dc++) {
                int r = tr + dr, c = tc + dc;
                if (r < 0 || r >= SCREEN_TILES_Y || c < 0 || c >= SCREEN_TILES_X) continue;
                if (game->current_screen.tiles[r][c] == TILE_BOMBABLE_WALL) {
                    game->current_screen.tiles[r][c] = TILE_FLOOR;
                }
            }
        }

        vfx_spawn(VFX_EXPLOSION, (Vector2){ cx, cy }, BOMB_BLAST_RADIUS);
        proj->active = false;
    }
}

static void check_combat(Game *game) {
    Player *p = &game->player;

    if (p->state == PSTATE_ATTACKING) {
        Rectangle sword = player_sword_hitbox(p);
        if (sword.width > 0) {
            for (int i = 0; i < game->enemy_count; i++) {
                Enemy *e = &game->enemies[i];
                if (!e->active || e->invuln_timer > 0) continue;
                if (CheckCollisionRecs(sword, enemy_hitbox(e))) {
                    e->invuln_timer = SWORD_ACTIVE_FRAMES;
                    sound_play(game, SOUND_SWORD_HIT);
                    if (e->type == ENEMY_SLIME && e->subtype == 0 &&
                        p->inventory.sword_tier <= 1) {
                        Vector2 split_pos = e->pos;
                        e->active = false;
                        e->state = ESTATE_DEAD;
                        sound_play(game, SOUND_ENEMY_DEATH);
                        spawn_small_slimes(game, split_pos);
                    } else {
                        enemy_take_damage(e, SWORD_DAMAGE);
                        if (!e->active) {
                            sound_play(game, SOUND_ENEMY_DEATH);
                            try_spawn_drop(game, e->pos);
                        }
                    }
                }
            }
        }
    }

    // Player projectiles vs enemies
    for (int pi = 0; pi < game->projectile_count; pi++) {
        Projectile *proj = &game->projectiles[pi];
        if (!proj->active || proj->owner != OWNER_PLAYER) continue;
        Rectangle proj_rect = projectile_hitbox(proj);
        for (int i = 0; i < game->enemy_count; i++) {
            Enemy *e = &game->enemies[i];
            if (!e->active || e->invuln_timer > 0) continue;
            if (!CheckCollisionRecs(proj_rect, enemy_hitbox(e))) continue;

            if (e->type == ENEMY_SLIME && e->subtype == 0 &&
                p->inventory.sword_tier <= 1 && proj->type == PROJ_ARROW) {
                Vector2 split_pos = e->pos;
                e->active = false;
                e->state = ESTATE_DEAD;
                sound_play(game, SOUND_ENEMY_DEATH);
                spawn_small_slimes(game, split_pos);
            } else {
                enemy_take_damage(e, proj->damage);
                e->invuln_timer = SWORD_ACTIVE_FRAMES;
                if (!e->active) {
                    sound_play(game, SOUND_ENEMY_DEATH);
                    try_spawn_drop(game, e->pos);
                } else {
                    sound_play(game, SOUND_SWORD_HIT);
                }
            }

            if (proj->stun_frames > 0 && e->active) {
                e->state = ESTATE_IDLE;
                e->state_timer = proj->stun_frames;
                e->velocity = (Vector2){0, 0};
            }
            if (proj->type != PROJ_BOOMERANG) proj->active = false;
            break;
        }
    }

    if (p->invuln_timer <= 0 && p->state != PSTATE_KNOCKBACK) {
        Rectangle player_rect = player_hitbox(p);
        for (int i = 0; i < game->enemy_count; i++) {
            Enemy *e = &game->enemies[i];
            if (!e->active) continue;
            if (CheckCollisionRecs(player_rect, enemy_hitbox(e))) {
                float dx = p->pos.x - e->pos.x;
                float dy = p->pos.y - e->pos.y;
                Direction kb_dir;
                if (fabsf(dx) > fabsf(dy))
                    kb_dir = (dx > 0) ? DIR_E : DIR_W;
                else
                    kb_dir = (dy > 0) ? DIR_S : DIR_N;
                player_take_damage(p, enemy_defs[e->type].contact_damage, kb_dir);
                sound_play(game, SOUND_PLAYER_DAMAGE);
                break;
            }
        }
    }

    // Enemy projectiles vs player
    if (p->invuln_timer <= 0 && p->state != PSTATE_KNOCKBACK) {
        Rectangle player_rect = player_hitbox(p);
        for (int pi = 0; pi < game->projectile_count; pi++) {
            Projectile *proj = &game->projectiles[pi];
            if (!proj->active || proj->owner != OWNER_ENEMY) continue;
            if (!CheckCollisionRecs(player_rect, projectile_hitbox(proj))) continue;

            bool blocked = false;
            if (p->state != PSTATE_ATTACKING && p->inventory.shield_tier >= 1) {
                Direction proj_from = opposite_dir(proj->facing);
                if (proj_from == p->facing) {
                    const ProjectileDef *pdef = &projectile_defs[proj->type];
                    if (pdef->blocked_by_shield_small ||
                        (p->inventory.shield_tier >= 2 && pdef->blocked_by_shield_large)) {
                        blocked = true;
                    }
                }
            }

            if (blocked) {
                proj->active = false;
                sound_play(game, SOUND_SHIELD_BLOCK);
            } else {
                player_take_damage(p, proj->damage, proj->facing);
                sound_play(game, SOUND_PLAYER_DAMAGE);
                proj->active = false;
            }
            break;
        }
    }
}

void game_init(Game *game) {
    memset(game, 0, sizeof(*game));
    game->state = STATE_PLAY;

    player_init(&game->player);
    load_screen_at(game, START_SCREEN_X, START_SCREEN_Y);
}

void game_update(Game *game) {
    if (IsKeyPressed(KEY_F3)) debug_toggle();

    float dt = GetFrameTime();

    if (game->music_loaded) {
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
            enemies_update(game->enemies, game->enemy_count,
                           game->player.pos, &game->current_screen,
                           game->projectiles, &game->projectile_count, dt);
            projectiles_update(game->projectiles, &game->projectile_count,
                               &game->current_screen, game->player.pos, dt);
            check_bomb_explosions(game);
            vfx_update();
            check_combat(game);
            pickups_update(game->pickups, game->pickup_count);
            check_pickups(game);
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
            check_warp(game);
            if (game->state == STATE_PLAY) {
                check_edge_transition(game);
            }
            break;
        }

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
                        game->player.pos.x = (float)(rw->tile_col * TILE_SIZE);
                        game->player.pos.y = (float)(PLAY_AREA_Y + (rw->tile_row - 1) * TILE_SIZE);
                    } else {
                        game->player.pos.x = 7.0f * TILE_SIZE;
                        game->player.pos.y = PLAY_AREA_Y + 5.0f * TILE_SIZE;
                    }
                } else if (game->in_cave) {
                    // Returning from cave to overworld
                    load_screen_at(game, game->warp_dest_x, game->warp_dest_y);
                    game->in_cave = false;
                    game->player.pos.x = (float)(game->return_tile_col * TILE_SIZE);
                    game->player.pos.y = (float)(PLAY_AREA_Y + (game->return_tile_row - 1) * TILE_SIZE);
                } else {
                    // Overworld-to-overworld warp
                    load_screen_at(game, game->warp_dest_x, game->warp_dest_y);
                    game->player.pos.x = 7.0f * TILE_SIZE;
                    game->player.pos.y = PLAY_AREA_Y + 5.0f * TILE_SIZE;
                }
            }

            if (!camera_is_active(&game->cam)) {
                if (game->trans_type == TRANS_SCROLL) {
                    game->current_screen = game->next_screen;
                    game->screen_x = game->warp_dest_x;
                    game->screen_y = game->warp_dest_y;
                    game->player.pos = game->trans_player_end;
                    enemies_spawn(game->enemies, &game->enemy_count,
                                  &game->current_screen);
                    projectiles_clear(game->projectiles, &game->projectile_count);
                    pickups_clear(game->pickups, &game->pickup_count);
                    vfx_clear();
                }
                game->state = STATE_PLAY;
            }
            break;

        case STATE_DEATH:
            game->death_timer--;
            if (game->death_timer <= 0) {
                player_init(&game->player);
                game->in_cave = false;
                load_screen_at(game, START_SCREEN_X, START_SCREEN_Y);
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
        pause_screen_draw(&game->pause_state, &game->player.inventory,
                          game->player.inventory.sword_tier);
    } else {
        screen_draw(&game->current_screen);
        enemies_draw(game->enemies, game->enemy_count);
        pickups_draw(game->pickups, game->pickup_count);
        projectiles_draw(game->projectiles, game->projectile_count);
        player_draw(&game->player);
        vfx_draw();
    }

    hud_draw(&game->player, game->screen_x, game->screen_y);
    debug_draw_game(game);
    debug_draw_overlay();

    EndDrawing();
}

#include "game.h"
#include "mario.h"
#include "camera.h"
#include "sounds.h"
#include "items.h"
#include "enemies/firebar.h"

static void start_level_at(Game *game, int world, int sublevel, int spawn_tx, int spawn_ty) {
    memset(game->entities, 0, sizeof(game->entities));
    game->mario = -1;

    level_free(&game->level);
    game->world = world;
    game->sublevel = sublevel;
    level_load(&game->level, world, sublevel);

    float sx = (float)(spawn_tx * TILE_SIZE);
    float sy = (float)(spawn_ty * TILE_SIZE - MARIO_SMALL_H);
    spawn_mario(game->entities, &game->mario, sx, sy);

    Entity *m = &game->entities[game->mario];
    if (game->saved_power >= MARIO_BIG) {
        m->power = game->saved_power;
        if (m->power >= MARIO_BIG) {
            m->y -= (MARIO_BIG_H - MARIO_SMALL_H);
            m->h = MARIO_BIG_H;
        }
    }

    game->camera_x = 0;
    game->timer = LEVEL_TIME;
    game->state = STATE_PLAYING;
    game->state_timer = 0;
}

static void start_level(Game *game) {
    start_level_at(game, game->world, game->sublevel, 3, 11);
}

void game_init(Game *game) {
    memset(game, 0, sizeof(Game));
    game->state = STATE_TITLE;
    game->lives = 3;
    game->world = 1;
    game->sublevel = 1;
    game->mario = -1;
}

// --- Title ---

static void update_title(Game *game) {
    if (IsKeyPressed(KEY_ENTER) ||
        (IsGamepadAvailable(0) && (IsGamepadButtonPressed(0, GAMEPAD_BUTTON_RIGHT_FACE_DOWN) ||
                                   IsGamepadButtonPressed(0, GAMEPAD_BUTTON_RIGHT_FACE_RIGHT)))) {
        start_level(game);
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

// --- Playing ---

static void update_playing(Game *game) {
    float dt = GetFrameTime();
    if (dt <= 0) return;

    // Pause
    if (IsKeyPressed(KEY_ESCAPE) || IsKeyPressed(KEY_P) ||
        (IsGamepadAvailable(0) && IsGamepadButtonPressed(0, GAMEPAD_BUTTON_MIDDLE_RIGHT))) {
        game->state = STATE_PAUSED;
        return;
    }

    Entity *mario = &game->entities[game->mario];

    // 0. Activate enemies that scrolled into view + Bill Blasters
    level_activate_spawns(&game->level, game->entities, game->camera_x);
    level_update_blasters(&game->level, game->entities, game->mario, game->camera_x, &game->blaster_timer);

    // 1. Mario update (input + velocity computation)
    if (mario->vtab && mario->vtab->update)
        mario->vtab->update(mario, game);

    // 2. Mario split-axis collision
    mario->x += mario->vx * dt;
    level_collide_x(&game->level, mario);
    mario->y += mario->vy * dt;
    level_collide_y(&game->level, mario, game);

    // Bail if we entered dying state during collision
    if (game->state != STATE_PLAYING) return;

    // 3. Star timer
    if (mario->star_active) {
        mario->star_timer -= dt;
        if (mario->star_timer <= 0) {
            mario->star_active = false;
            mario->star_timer = 0;
        }
    }

    // 4. Non-Mario entity update + tile collision
    for (int i = 0; i < MAX_ENTITIES; i++) {
        if (game->entities[i].type == ENT_NONE || i == game->mario) continue;
        Entity *e = &game->entities[i];

        // Dead-falling entities
        if (e->dead_falling) {
            e->vy += GRAVITY * dt;
            e->y += e->vy * dt;
            if (e->y > game->level.height * TILE_SIZE + 200)
                entity_deactivate(e);
            continue;
        }

        if (e->vtab && e->vtab->update)
            e->vtab->update(e, game);

        if (e->self_moving) continue;

        e->x += e->vx * dt;
        level_collide_x(&game->level, e);
        e->y += e->vy * dt;
        level_collide_y(&game->level, e, game);
    }

    // 5. Entity-vs-Mario collision
    for (int i = 0; i < MAX_ENTITIES; i++) {
        Entity *e = &game->entities[i];
        if (e->type == ENT_NONE || i == game->mario) continue;
        if (e->type == ENT_BALANCE_LIFT || e->type == ENT_FIREBAR) continue;

        if (!entity_overlap(mario, e)) continue;

        // Star: kill enemies on contact
        if (mario->star_active && e->star_killable) {
            if (e->vtab && e->vtab->hit_by_star) {
                e->vtab->hit_by_star(e, game);
                spawn_score_popup(game->entities, e->x, e->y, SCORE_GOOMBA_STOMP);
            }
            continue;
        }

        // Items (non-damaging): collect via touch
        if (!e->damages_mario && e->vtab && e->vtab->touch) {
            e->vtab->touch(e, mario, game);
            continue;
        }

        // Stomp check
        if (e->stompable && mario_is_stomping(mario, e)) {
            if (e->vtab && e->vtab->stomped)
                e->vtab->stomped(e, mario, game);
            mario->vy = MARIO_STOMP_BOUNCE;
            spawn_score_popup(game->entities, e->x, e->y - 16, SCORE_GOOMBA_STOMP);
            continue;
        }

        // Shell: kick stationary shell
        if (e->type == ENT_SHELL && fabsf(e->vx) < 1.0f && e->vtab && e->vtab->touch) {
            e->vtab->touch(e, mario, game);
            continue;
        }

        // Damage Mario
        if (e->damages_mario) {
            mario_take_damage(mario, game);
            if (game->state != STATE_PLAYING) return;
        }
    }

    // 6. Entity-vs-entity collision (shell & fireball vs enemies)
    for (int i = 0; i < MAX_ENTITIES; i++) {
        Entity *a = &game->entities[i];
        if (a->type == ENT_NONE) continue;
        bool a_is_shell = (a->type == ENT_SHELL && fabsf(a->vx) > 1.0f);
        bool a_is_fireball = (a->type == ENT_FIREBALL);
        if (!a_is_shell && !a_is_fireball) continue;

        for (int j = 0; j < MAX_ENTITIES; j++) {
            if (i == j) continue;
            Entity *b = &game->entities[j];
            if (b->type == ENT_NONE || j == game->mario) continue;
            if (!b->destructible) continue;

            if (!entity_overlap(a, b)) continue;

            if (a_is_fireball && b->type != ENT_SHELL) {
                if (b->fire_immune) {
                    if (b->vtab && b->vtab->hit_by_fire)
                        b->vtab->hit_by_fire(b, game);
                    continue;
                }
                if (b->vtab && b->vtab->hit_by_fire) {
                    b->vtab->hit_by_fire(b, game);
                    spawn_score_popup(game->entities, b->x, b->y - 16, SCORE_FIREBALL_KILL);
                }
                entity_deactivate(a);
                break;
            }

            if (a_is_shell && b->type != ENT_SHELL) {
                if (b->shell_killable && b->vtab && b->vtab->hit_by_shell) {
                    b->vtab->hit_by_shell(b, game);
                    spawn_score_popup(game->entities, b->x, b->y - 16, SCORE_SHELL_KILL);
                }
            }

            // Shell vs shell
            if (a_is_shell && b->type == ENT_SHELL) {
                if (b->vtab && b->vtab->hit_by_shell)
                    b->vtab->hit_by_shell(b, game);
                entity_deactivate(a);
                break;
            }
        }
    }

    // 6b. Firebar collision (per-ball check, logic lives in firebar.c)
    for (int i = 0; i < MAX_ENTITIES; i++) {
        Entity *e = &game->entities[i];
        if (e->type != ENT_FIREBAR) continue;
        if (mario->star_active || mario->invincible_timer > 0) continue;
        if (firebar_overlaps_entity(e, mario)) {
            mario_take_damage(mario, game);
            if (game->state != STATE_PLAYING) return;
        }
    }

    // 7. Camera
    camera_update(&game->camera_x, mario, &game->level);

    // 8. Timer countdown
    game->timer -= TIMER_TICK_RATE * dt;
    if (game->timer <= 0) {
        game->timer = 0;
        game->state = STATE_DYING;
        game->state_timer = 0;
        sound_play(SND_DEATH);
    }

    // 9. Particles
    particles_update(game->particles, dt);

    // 10. Flagpole check
    int mario_tx = (int)((mario->x + mario->w / 2) / TILE_SIZE);
    int mario_ty = (int)((mario->y + mario->h / 2) / TILE_SIZE);
    int tile = level_get_tile(&game->level, mario_tx, mario_ty);
    if (tile == TILE_FLAGPOLE || tile == TILE_FLAGPOLE_BASE) {
        game->state = STATE_LEVEL_COMPLETE;
        game->state_timer = 0;
        game->score += (int)game->timer * 50;
        game->timer = 0;
        game->saved_power = mario->power;
        sound_play(SND_FLAGPOLE);
    }

    // 11. Axe check (castle levels)
    if (tile == TILE_AXE) {
        game->state = STATE_CASTLE_COMPLETE;
        game->state_timer = 0;
        game->saved_power = mario->power;
        game->bridge_collapse_tx = game->level.bridge_end_tx;
        mario->vx = 0;
        mario->vy = 0;
    }

    // 12. Lava death
    {
        int feet_tx = (int)((mario->x + mario->w / 2) / TILE_SIZE);
        int feet_ty = (int)((mario->y + mario->h - 1) / TILE_SIZE);
        if (level_get_tile(&game->level, feet_tx, feet_ty) == TILE_LAVA) {
            game->state = STATE_DYING;
            game->state_timer = 0;
            sound_play(SND_DEATH);
        }
    }

    // 13. Pipe entry check
    if (mario->on_ground && (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S) ||
        (IsGamepadAvailable(0) && IsGamepadButtonPressed(0, GAMEPAD_BUTTON_LEFT_FACE_DOWN)))) {
        int pipe_tx = (int)((mario->x + mario->w / 2) / TILE_SIZE);
        int pipe_ty = (int)((mario->y + mario->h) / TILE_SIZE) - 1;
        int t = level_get_tile(&game->level, pipe_tx, pipe_ty);
        if (t == TILE_PIPE_TL || t == TILE_PIPE_TR) {
            int left_tx = (t == TILE_PIPE_TR) ? pipe_tx - 1 : pipe_tx;
            PipeWarp *w = level_get_warp(&game->level, left_tx, pipe_ty);
            if (w) {
                game->warp_world = w->dest_world;
                game->warp_sublevel = w->dest_sublevel;
                game->warp_tx = w->dest_tx;
                game->warp_ty = w->dest_ty;
                game->state = STATE_PIPE_TRANSITION;
                game->state_timer = 0;
                game->saved_power = mario->power;
                sound_play(SND_PIPE);
            }
        }
    }
}

static void draw_playing(Game *game) {
    level_draw(&game->level, game->camera_x);

    for (int i = 0; i < MAX_ENTITIES; i++) {
        Entity *e = &game->entities[i];
        if (e->type == ENT_NONE) continue;
        if (e->vtab && e->vtab->draw)
            e->vtab->draw(e, game->camera_x);
    }

    particles_draw(game->particles);
}

// --- Dying ---

static void update_dying(Game *game) {
    float dt = GetFrameTime();
    game->state_timer += dt;

    Entity *mario = &game->entities[game->mario];

    if (game->state_timer < 0.4f) {
        mario->vy = MARIO_JUMP_VEL * 0.6f;
    } else {
        mario->vy += GRAVITY * dt;
        if (mario->vy > MAX_FALL_SPEED) mario->vy = MAX_FALL_SPEED;
    }
    mario->y += mario->vy * dt;

    if (game->state_timer >= DEATH_ANIM_TIME) {
        game->lives--;
        game->saved_power = MARIO_SMALL;
        if (game->lives <= 0) {
            game->state = STATE_GAME_OVER;
            game->state_timer = 0;
        } else {
            start_level(game);
        }
    }
}

static void draw_dying(Game *game) {
    level_draw(&game->level, game->camera_x);

    Entity *mario = &game->entities[game->mario];
    if (mario->vtab && mario->vtab->draw)
        mario->vtab->draw(mario, game->camera_x);
}

// --- Game Over ---

static void update_game_over(Game *game) {
    float dt = GetFrameTime();
    game->state_timer += dt;

    if (game->state_timer >= GAME_OVER_TIME) {
        game_init(game);
    }
}

static void draw_game_over(Game *game) {
    (void)game;
    const char *text = "GAME OVER";
    int tw = MeasureText(text, 40);
    DrawText(text, (WINDOW_WIDTH - tw) / 2, WINDOW_HEIGHT / 2 - 20, 40, COLOR_TEXT);
}

// --- Paused ---

static void update_paused(Game *game) {
    if (IsKeyPressed(KEY_ESCAPE) || IsKeyPressed(KEY_P) ||
        (IsGamepadAvailable(0) && IsGamepadButtonPressed(0, GAMEPAD_BUTTON_MIDDLE_RIGHT))) {
        game->state = STATE_PLAYING;
    }
}

static void draw_paused(Game *game) {
    draw_playing(game);
    DrawRectangle(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, (Color){0, 0, 0, 128});
    const char *text = "PAUSED";
    int tw = MeasureText(text, 40);
    DrawText(text, (WINDOW_WIDTH - tw) / 2, WINDOW_HEIGHT / 2 - 20, 40, COLOR_TEXT);
}

// --- Level Complete ---

static void advance_to_next_level(Game *game) {
    if (game->sublevel < 4) {
        game->sublevel++;
    } else {
        game->world++;
        game->sublevel = 1;
    }
    start_level(game);
}

static void update_level_complete(Game *game) {
    float dt = GetFrameTime();
    game->state_timer += dt;

    Entity *mario = &game->entities[game->mario];
    mario->vx = MARIO_WALK_SPEED * 0.5f;
    mario->x += mario->vx * dt;
    mario->vy += GRAVITY * dt;
    if (mario->vy > MAX_FALL_SPEED) mario->vy = MAX_FALL_SPEED;
    mario->y += mario->vy * dt;
    level_collide_entity(&game->level, mario, game);

    if (game->state_timer >= LEVEL_COMPLETE_TIME) {
        advance_to_next_level(game);
    }
}

static void draw_level_complete(Game *game) {
    draw_playing(game);
    const char *text = "LEVEL COMPLETE!";
    int tw = MeasureText(text, 40);
    DrawText(text, (WINDOW_WIDTH - tw) / 2, WINDOW_HEIGHT / 3, 40, COLOR_TEXT);
}

// --- Castle Complete ---

static void update_castle_complete(Game *game) {
    float dt = GetFrameTime();
    game->state_timer += dt;

    // Phase 1: collapse bridge tiles one by one
    if (game->bridge_collapse_tx >= game->level.bridge_start_tx) {
        float collapse_elapsed = game->state_timer;
        int tiles_collapsed = (int)(collapse_elapsed / BRIDGE_COLLAPSE_RATE);
        int target_tx = game->level.bridge_end_tx - tiles_collapsed;
        while (game->bridge_collapse_tx >= target_tx &&
               game->bridge_collapse_tx >= game->level.bridge_start_tx) {
            level_set_tile(&game->level, game->bridge_collapse_tx, game->level.bridge_ty, TILE_EMPTY);
            game->bridge_collapse_tx--;
        }
    }

    // Make Bowser fall if bridge gone
    for (int i = 0; i < MAX_ENTITIES; i++) {
        Entity *e = &game->entities[i];
        if (e->type == ENT_BOWSER) {
            e->vy += GRAVITY * dt;
            if (e->vy > MAX_FALL_SPEED) e->vy = MAX_FALL_SPEED;
            e->y += e->vy * dt;
            if (e->y > game->level.height * TILE_SIZE + 200) {
                entity_deactivate(e);
                sound_play(SND_BOWSER_FALL);
            }
        }
    }

    // Phase 2: show message after bridge fully collapsed
    float bridge_tiles = (float)(game->level.bridge_end_tx - game->level.bridge_start_tx + 1);
    float total_time = bridge_tiles * BRIDGE_COLLAPSE_RATE + CASTLE_MESSAGE_TIME;
    if (game->state_timer >= total_time) {
        game->score += (int)game->timer * 50;
        game->timer = 0;
        if (game->world == 8 && game->sublevel == 4) {
            game->state = STATE_WIN;
            game->state_timer = 0;
        } else {
            advance_to_next_level(game);
        }
    }
}

static void draw_castle_complete(Game *game) {
    draw_playing(game);

    float bridge_tiles = (float)(game->level.bridge_end_tx - game->level.bridge_start_tx + 1);
    float msg_start = bridge_tiles * BRIDGE_COLLAPSE_RATE;
    if (game->state_timer >= msg_start) {
        const char *text = "THANK YOU MARIO!";
        int tw = MeasureText(text, 30);
        DrawText(text, (WINDOW_WIDTH - tw) / 2, WINDOW_HEIGHT / 3, 30, COLOR_TEXT);

        const char *text2 = "BUT OUR PRINCESS IS IN ANOTHER CASTLE!";
        int tw2 = MeasureText(text2, 20);
        DrawText(text2, (WINDOW_WIDTH - tw2) / 2, WINDOW_HEIGHT / 3 + 50, 20, COLOR_TEXT);
    }
}

// --- Win ---

#define WIN_DISPLAY_TIME 8.0f

static void update_win(Game *game) {
    float dt = GetFrameTime();
    game->state_timer += dt;
    if (game->state_timer >= WIN_DISPLAY_TIME) {
        game_init(game);
    }
}

static void draw_win(Game *game) {
    (void)game;
    const char *t1 = "THANK YOU MARIO!";
    int tw1 = MeasureText(t1, 40);
    DrawText(t1, (WINDOW_WIDTH - tw1) / 2, WINDOW_HEIGHT / 3, 40, COLOR_TEXT);

    const char *t2 = "YOUR QUEST IS OVER.";
    int tw2 = MeasureText(t2, 30);
    DrawText(t2, (WINDOW_WIDTH - tw2) / 2, WINDOW_HEIGHT / 3 + 60, 30, COLOR_TEXT);

    const char *t3 = "WE PRESENT YOU A NEW QUEST.";
    int tw3 = MeasureText(t3, 20);
    DrawText(t3, (WINDOW_WIDTH - tw3) / 2, WINDOW_HEIGHT / 3 + 110, 20, COLOR_TEXT);

    const char *t4 = "PRESS ENTER";
    int tw4 = MeasureText(t4, 20);
    DrawText(t4, (WINDOW_WIDTH - tw4) / 2, WINDOW_HEIGHT / 3 + 170, 20, COLOR_TEXT);
}

// --- Pipe Transition ---

static void update_pipe_transition(Game *game) {
    float dt = GetFrameTime();
    game->state_timer += dt;

    // Mario sinks into pipe during first half
    if (game->state_timer < PIPE_TRANSITION_TIME / 2) {
        Entity *mario = &game->entities[game->mario];
        mario->y += 120.0f * dt;
    }

    if (game->state_timer >= PIPE_TRANSITION_TIME) {
        // Same world/sublevel = reposition within level
        if (game->warp_world == game->world && game->warp_sublevel == game->sublevel) {
            Entity *mario = &game->entities[game->mario];
            mario->x = (float)(game->warp_tx * TILE_SIZE);
            mario->y = (float)(game->warp_ty * TILE_SIZE) - mario->h;
            mario->vx = 0;
            mario->vy = 0;
            game->camera_x = mario->x - CAMERA_THRESHOLD;
            if (game->camera_x < 0) game->camera_x = 0;
            game->state = STATE_PLAYING;
        } else {
            start_level_at(game, game->warp_world, game->warp_sublevel,
                          game->warp_tx, game->warp_ty);
        }
    }
}

static void draw_pipe_transition(Game *game) {
    float t = game->state_timer / PIPE_TRANSITION_TIME;
    if (t < 0.5f) {
        draw_playing(game);
        float fade = t * 2.0f;
        DrawRectangle(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT,
                      (Color){0, 0, 0, (unsigned char)(fade * 255)});
    } else {
        float fade = (1.0f - t) * 2.0f;
        DrawRectangle(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT,
                      (Color){0, 0, 0, (unsigned char)(fade * 255)});
    }
}

// --- HUD ---

static void draw_hud(Game *game) {
    DrawText("MARIO", 40, 10, 20, COLOR_TEXT);
    DrawText(TextFormat("%06d", game->score), 40, 30, 20, COLOR_TEXT);

    DrawText(TextFormat("x%02d", game->coins), 350, 30, 20, COLOR_TEXT);

    DrawText("WORLD", 560, 10, 20, COLOR_TEXT);
    DrawText(TextFormat(" %d-%d", game->world, game->sublevel), 560, 30, 20, COLOR_TEXT);

    DrawText("TIME", 800, 10, 20, COLOR_TEXT);
    DrawText(TextFormat(" %03d", (int)game->timer), 800, 30, 20, COLOR_TEXT);

    DrawText("LIVES", 1020, 10, 20, COLOR_TEXT);
    DrawText(TextFormat("  x%d", game->lives), 1020, 30, 20, COLOR_TEXT);
}

// --- Main dispatch ---

void game_update(Game *game) {
    switch (game->state) {
        case STATE_TITLE:           update_title(game);           break;
        case STATE_PLAYING:         update_playing(game);         break;
        case STATE_DYING:           update_dying(game);           break;
        case STATE_GAME_OVER:       update_game_over(game);       break;
        case STATE_PAUSED:          update_paused(game);          break;
        case STATE_LEVEL_COMPLETE:  update_level_complete(game);  break;
        case STATE_CASTLE_COMPLETE: update_castle_complete(game); break;
        case STATE_PIPE_TRANSITION: update_pipe_transition(game); break;
        case STATE_WIN:             update_win(game);             break;
    }
}

void game_draw(Game *game) {
    BeginDrawing();
    bool in_level = (game->state == STATE_PLAYING || game->state == STATE_PAUSED ||
                     game->state == STATE_DYING || game->state == STATE_LEVEL_COMPLETE ||
                     game->state == STATE_CASTLE_COMPLETE || game->state == STATE_PIPE_TRANSITION);
    ClearBackground(in_level ? game->level.bg_color : COLOR_BG);

    switch (game->state) {
        case STATE_TITLE:
            draw_title(game);
            break;
        case STATE_PLAYING:
            draw_hud(game);
            draw_playing(game);
            break;
        case STATE_DYING:
            draw_hud(game);
            draw_dying(game);
            break;
        case STATE_GAME_OVER:
            draw_game_over(game);
            break;
        case STATE_PAUSED:
            draw_hud(game);
            draw_paused(game);
            break;
        case STATE_LEVEL_COMPLETE:
            draw_hud(game);
            draw_level_complete(game);
            break;
        case STATE_CASTLE_COMPLETE:
            draw_hud(game);
            draw_castle_complete(game);
            break;
        case STATE_PIPE_TRANSITION:
            draw_hud(game);
            draw_pipe_transition(game);
            break;
        case STATE_WIN:
            draw_win(game);
            break;
    }

    EndDrawing();
}

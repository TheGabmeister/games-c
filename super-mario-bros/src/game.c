#include "game.h"
#include "mario.h"
#include "camera.h"
#include "sounds.h"
#include "items.h"

static void start_level(Game *game) {
    memset(game->entities, 0, sizeof(game->entities));
    game->mario = -1;

    level_free(&game->level);
    level_load_1_1(&game->level);

    Entity *m_ent = NULL;
    spawn_mario(game->entities, &game->mario, 3 * TILE_SIZE, 11 * TILE_SIZE - MARIO_SMALL_H);
    m_ent = &game->entities[game->mario];
    // Preserve power state across respawns within a session
    (void)m_ent;

    game->camera_x = 0;
    game->timer = LEVEL_TIME;
    game->state = STATE_PLAYING;
    game->state_timer = 0;
}

void game_init(Game *game) {
    memset(game, 0, sizeof(Game));
    game->state = STATE_TITLE;
    game->lives = 3;
    game->world = 1;
    game->sublevel = 1;
    game->mario = -1;
}

// --- Helpers ---

static bool mario_is_stomping(Entity *mario, Entity *enemy) {
    // Mario is falling and his bottom overlaps enemy's top half
    return mario->vy > 0 &&
           (mario->y + mario->h) > enemy->y &&
           (mario->y + mario->h) < enemy->y + enemy->h * 0.6f;
}

static void mario_take_damage(Entity *mario, Game *game) {
    if (mario->invincible_timer > 0 || mario->star_active) return;

    if (mario->power > MARIO_SMALL) {
        mario->power = MARIO_SMALL;
        mario->y += (mario->h - MARIO_SMALL_H);
        mario->h = MARIO_SMALL_H;
        mario->invincible_timer = DAMAGE_INVINCIBLE_TIME;
        sound_play(SND_BUMP);
    } else {
        game->state = STATE_DYING;
        game->state_timer = 0;
        sound_play(SND_DEATH);
    }
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

    // 0. Activate enemies that scrolled into view
    level_activate_spawns(&game->level, game->entities, game->camera_x);

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

        // Squished goomba timer
        if (e->type == ENT_GOOMBA && e->state_val == 1) {
            e->state_timer -= dt;
            if (e->state_timer <= 0) {
                entity_deactivate(e);
                continue;
            }
        }
        // Dead-falling entities (state_val == 2)
        if (e->state_val == 2) {
            e->vy += GRAVITY * dt;
            e->y += e->vy * dt;
            if (e->y > game->level.height * TILE_SIZE + 200)
                entity_deactivate(e);
            continue;
        }

        if (e->vtab && e->vtab->update)
            e->vtab->update(e, game);

        // Items rising from blocks skip tile collision
        if ((e->type == ENT_MUSHROOM || e->type == ENT_FIRE_FLOWER ||
             e->type == ENT_STARMAN || e->type == ENT_ONEUP) && e->state_val == 0)
            continue;

        // Debris/popups skip tile collision
        if (e->type == ENT_BRICK_DEBRIS || e->type == ENT_COIN_POPUP || e->type == ENT_SCORE_POPUP)
            continue;

        e->x += e->vx * dt;
        level_collide_x(&game->level, e);
        e->y += e->vy * dt;
        level_collide_y(&game->level, e, game);
    }

    // 5. Entity-vs-Mario collision
    for (int i = 0; i < MAX_ENTITIES; i++) {
        Entity *e = &game->entities[i];
        if (e->type == ENT_NONE || i == game->mario) continue;

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
            if (b->type == ENT_BRICK_DEBRIS || b->type == ENT_COIN_POPUP ||
                b->type == ENT_SCORE_POPUP || b->type == ENT_FIREBALL) continue;

            if (!entity_overlap(a, b)) continue;

            if (a_is_fireball && b->type != ENT_SHELL) {
                if (!b->fire_immune && b->vtab && b->vtab->hit_by_fire) {
                    b->vtab->hit_by_fire(b, game);
                    spawn_score_popup(game->entities, b->x, b->y - 16, SCORE_FIREBALL_KILL);
                }
                entity_deactivate(a);
                break;
            }

            if (a_is_shell && b->type != ENT_SHELL && b->type != ENT_FIREBALL) {
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
        start_level(game);
    }
}

static void draw_level_complete(Game *game) {
    draw_playing(game);
    const char *text = "LEVEL COMPLETE!";
    int tw = MeasureText(text, 40);
    DrawText(text, (WINDOW_WIDTH - tw) / 2, WINDOW_HEIGHT / 3, 40, COLOR_TEXT);
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
        case STATE_TITLE:          update_title(game);          break;
        case STATE_PLAYING:        update_playing(game);        break;
        case STATE_DYING:          update_dying(game);          break;
        case STATE_GAME_OVER:      update_game_over(game);      break;
        case STATE_PAUSED:         update_paused(game);         break;
        case STATE_LEVEL_COMPLETE: update_level_complete(game);  break;
    }
}

void game_draw(Game *game) {
    BeginDrawing();
    ClearBackground(game->state == STATE_PLAYING || game->state == STATE_PAUSED ||
                    game->state == STATE_DYING || game->state == STATE_LEVEL_COMPLETE
                    ? game->level.bg_color : COLOR_BG);

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
    }

    EndDrawing();
}

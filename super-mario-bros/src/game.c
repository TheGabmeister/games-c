#include "game.h"
#include "mario.h"
#include "camera.h"
#include "sounds.h"

static void start_level(Game *game) {
    // Clear entities
    memset(game->entities, 0, sizeof(game->entities));
    game->mario = -1;

    // Load level
    level_free(&game->level);
    level_load_test(&game->level);

    // Spawn Mario at tile (2, 11) — above ground
    spawn_mario(game->entities, &game->mario, 2 * TILE_SIZE, 11 * TILE_SIZE - MARIO_SMALL_H);

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

    // 1. Mario update (input + velocity computation)
    if (mario->vtab && mario->vtab->update)
        mario->vtab->update(mario, game);

    // 2. Mario split-axis collision: move X, collide X, move Y, collide Y
    mario->x += mario->vx * dt;
    level_collide_x(&game->level, mario);
    mario->y += mario->vy * dt;
    level_collide_y(&game->level, mario, game);

    // 3. Non-Mario entity update + tile collision
    for (int i = 0; i < MAX_ENTITIES; i++) {
        if (game->entities[i].type == ENT_NONE || i == game->mario) continue;
        Entity *e = &game->entities[i];
        if (e->vtab && e->vtab->update)
            e->vtab->update(e, game);
        e->x += e->vx * dt;
        level_collide_x(&game->level, e);
        e->y += e->vy * dt;
        level_collide_y(&game->level, e, game);
    }

    // 4. Camera
    camera_update(&game->camera_x, mario, &game->level);

    // 5. Timer countdown
    game->timer -= TIMER_TICK_RATE * dt;
    if (game->timer <= 0) {
        game->timer = 0;
        game->state = STATE_DYING;
        game->state_timer = 0;
        sound_play(SND_DEATH);
    }

    // 6. Particles
    particles_update(game->particles, dt);

    // 7. Flagpole check: if Mario reaches the flagpole
    int mario_tx = (int)((mario->x + mario->w / 2) / TILE_SIZE);
    int mario_ty = (int)((mario->y + mario->h / 2) / TILE_SIZE);
    int tile = level_get_tile(&game->level, mario_tx, mario_ty);
    if (tile == TILE_FLAGPOLE || tile == TILE_FLAGPOLE_BASE) {
        game->state = STATE_LEVEL_COMPLETE;
        game->state_timer = 0;
        // Timer bonus
        game->score += (int)game->timer * 50;
        game->timer = 0;
    }
}

static void draw_playing(Game *game) {
    level_draw(&game->level, game->camera_x);

    // Draw entities
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

    // Death bounce animation
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

    // Dim overlay
    DrawRectangle(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, (Color){0, 0, 0, 128});

    const char *text = "PAUSED";
    int tw = MeasureText(text, 40);
    DrawText(text, (WINDOW_WIDTH - tw) / 2, WINDOW_HEIGHT / 2 - 20, 40, COLOR_TEXT);
}

// --- Level Complete ---

static void update_level_complete(Game *game) {
    float dt = GetFrameTime();
    game->state_timer += dt;

    // Mario walks right automatically
    Entity *mario = &game->entities[game->mario];
    mario->vx = MARIO_WALK_SPEED * 0.5f;
    mario->x += mario->vx * dt;
    mario->vy += GRAVITY * dt;
    if (mario->vy > MAX_FALL_SPEED) mario->vy = MAX_FALL_SPEED;
    mario->y += mario->vy * dt;
    level_collide_entity(&game->level, mario, game);

    if (game->state_timer >= LEVEL_COMPLETE_TIME) {
        // For Phase 1, just restart the same level
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
    DrawText(TextFormat("MARIO"), 40, 10, 20, COLOR_TEXT);
    DrawText(TextFormat("%06d", game->score), 40, 30, 20, COLOR_TEXT);

    DrawText(TextFormat("x%02d", game->coins), 350, 30, 20, COLOR_TEXT);

    DrawText(TextFormat("WORLD"), 560, 10, 20, COLOR_TEXT);
    DrawText(TextFormat(" %d-%d", game->world, game->sublevel), 560, 30, 20, COLOR_TEXT);

    DrawText(TextFormat("TIME"), 800, 10, 20, COLOR_TEXT);
    DrawText(TextFormat(" %03d", (int)game->timer), 800, 30, 20, COLOR_TEXT);

    DrawText(TextFormat("LIVES"), 1020, 10, 20, COLOR_TEXT);
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

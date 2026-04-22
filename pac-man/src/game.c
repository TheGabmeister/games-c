#include "game.h"
#include <stdio.h>
#include <string.h>
#include <math.h>

// --- Data tables ---

static const float scatter_chase_times[SCATTER_CHASE_PHASES][3] = {
    { 7.0f,  7.0f,    5.0f    },
    { 20.0f, 20.0f,   20.0f   },
    { 7.0f,  7.0f,    5.0f    },
    { 20.0f, 20.0f,   20.0f   },
    { 5.0f,  5.0f,    5.0f    },
    { 20.0f, 1033.0f, 1037.0f },
    { 5.0f,  1.0f/60, 1.0f/60 },
    { 9999.0f, 9999.0f, 9999.0f },
};

static const float fright_duration[] = { 6,5,4,3,2,5,2,2,1,5,2,1,1,3,1,1,0,1,0 };
static const int fright_flashes[] = { 5,5,5,5,5,5,5,5,3,5,5,3,3,5,3,3,0,3,0 };
static const int global_dot_limits[GHOST_COUNT] = { -1, 7, 17, 32 };

static const float pacman_speed_eating[] = { 0.71f, 0.79f, 0.87f, 0.79f };

static const char *sound_files[SOUND_COUNT] = {
    "resources/dot_a.wav", "resources/dot_b.wav", "resources/power_pellet.wav",
    "resources/ghost_eaten.wav", "resources/fruit.wav", "resources/death.wav",
    "resources/extra_life.wav", "resources/ready.wav", "resources/menu_select.wav",
};

static const int elroy_thresholds[][2] = {
    {20,10}, {30,15}, {40,20}, {50,25}, {60,30}, {80,40}, {100,50}, {120,60}
};

// --- Utility ---

static int scatter_chase_tier(int level) {
    if (level <= 1) return 0;
    if (level <= 4) return 1;
    return 2;
}

static int fright_level_index(int level) {
    int idx = level - 1;
    if (idx < 0) idx = 0;
    if (idx >= 19) idx = 18;
    return idx;
}

static int elroy_tier(int level) {
    if (level <= 1) return 0;
    if (level <= 2) return 1;
    if (level <= 5) return 2;
    if (level <= 8) return 3;
    if (level <= 11) return 4;
    if (level <= 14) return 5;
    if (level <= 18) return 6;
    return 7;
}

static void play_sound(Game *game, SoundID id) {
    PlaySound(game->sounds[id]);
}

// --- Sound ---

void game_load_sounds(Game *game) {
    for (int i = 0; i < SOUND_COUNT; i++)
        game->sounds[i] = LoadSound(sound_files[i]);
}

void game_unload_sounds(Game *game) {
    for (int i = 0; i < SOUND_COUNT; i++)
        UnloadSound(game->sounds[i]);
}

// --- High score ---

void highscore_load(Game *game) {
    FILE *f = fopen("resources/highscore.dat", "rb");
    if (f) { fread(&game->high_score, sizeof(int), 1, f); fclose(f); }
}

void highscore_save(Game *game) {
    if (game->score > game->high_score) {
        game->high_score = game->score;
        FILE *f = fopen("resources/highscore.dat", "wb");
        if (f) { fwrite(&game->high_score, sizeof(int), 1, f); fclose(f); }
    }
}

// --- Initialization ---

static void init_dots(Game *game) {
    game->dots_remaining = 0;
    game->dots_eaten_this_level = 0;
    for (int r = 0; r < MAZE_ROWS; r++) {
        for (int c = 0; c < MAZE_COLS; c++) {
            int t = maze_layout[r][c];
            if (t == TILE_DOT || t == TILE_POWER_PELLET) {
                game->dot_eaten[r][c] = false;
                game->dots_remaining++;
            } else {
                game->dot_eaten[r][c] = true;
            }
        }
    }
    game->total_dots = game->dots_remaining;
}

static void reset_scatter_chase(Game *game) {
    game->global_mode = GLOBAL_SCATTER;
    game->scatter_chase_phase = 0;
    game->scatter_chase_timer = scatter_chase_times[0][scatter_chase_tier(game->level)];
}

static void game_reset_level(Game *game) {
    init_dots(game);
    pacman_init(&game->pacman);
    game->use_global_dot_counter = false;
    game->global_dot_counter = 0;
    ghost_init_all(game->ghosts, game->level, false);
    game->pellet_flash_timer = 0.0f;
    game->pellet_visible = true;
    game->frightened_active = false;
    game->frightened_timer = 0.0f;
    game->ghost_eat_combo = 0;
    game->ghost_eaten_pause = false;
    game->no_dot_timer = 0.0f;
    game->no_dot_timeout = (game->level <= 4) ? 4.0f : 3.0f;
    game->level_complete_flash_white = false;
    game->fruit.active = false;
    game->fruit.score_display = false;
    game->fruit_spawned_70 = false;
    game->fruit_spawned_170 = false;
    game->dot_sound_toggle = false;
    memset(game->particles, 0, sizeof(game->particles));
    reset_scatter_chase(game);
}

static void game_reset_positions(Game *game) {
    pacman_init(&game->pacman);
    game->use_global_dot_counter = true;
    game->global_dot_counter = 0;
    ghost_init_all(game->ghosts, game->level, true);
    game->frightened_active = false;
    game->frightened_timer = 0.0f;
    game->ghost_eat_combo = 0;
    game->ghost_eaten_pause = false;
    game->no_dot_timer = 0.0f;
    reset_scatter_chase(game);
}

void game_init(Game *game) {
    memset(game, 0, sizeof(Game));
    game->state = STATE_TITLE;
    game->level = 1;
    game->lives = 3;
    game->extra_life_given = false;
    highscore_load(game);
    game_reset_level(game);
}

// --- Scatter/Chase timing ---

static void update_scatter_chase(Game *game, float dt) {
    if (game->frightened_active) return;
    game->scatter_chase_timer -= dt;
    if (game->scatter_chase_timer <= 0.0f) {
        game->scatter_chase_phase++;
        if (game->scatter_chase_phase >= SCATTER_CHASE_PHASES)
            game->scatter_chase_phase = SCATTER_CHASE_PHASES - 1;

        game->global_mode = (game->scatter_chase_phase % 2 == 0) ? GLOBAL_SCATTER : GLOBAL_CHASE;
        game->scatter_chase_timer = scatter_chase_times[game->scatter_chase_phase][scatter_chase_tier(game->level)];

        for (int i = 0; i < GHOST_COUNT; i++) {
            Ghost *g = &game->ghosts[i];
            if (g->mode == GHOST_CHASE || g->mode == GHOST_SCATTER) {
                g->mode = (game->global_mode == GLOBAL_CHASE) ? GHOST_CHASE : GHOST_SCATTER;
                g->dir = direction_opposite(g->dir);
            }
        }

        int et = elroy_tier(game->level);
        if (game->dots_remaining <= elroy_thresholds[et][0])
            game->ghosts[GHOST_BLINKY].mode = GHOST_CHASE;
    }
}

// --- Frightened mode ---

static void activate_frightened(Game *game) {
    int idx = fright_level_index(game->level);
    float duration = fright_duration[idx];
    game->ghost_eat_combo = 0;

    if (duration <= 0.0f) {
        for (int i = 0; i < GHOST_COUNT; i++) {
            Ghost *g = &game->ghosts[i];
            if (g->mode == GHOST_CHASE || g->mode == GHOST_SCATTER)
                g->dir = direction_opposite(g->dir);
        }
        return;
    }

    game->frightened_active = true;
    game->frightened_timer = duration;
    game->frightened_duration = duration;
    game->frightened_flashes = fright_flashes[idx];

    for (int i = 0; i < GHOST_COUNT; i++) {
        Ghost *g = &game->ghosts[i];
        if (g->mode == GHOST_CHASE || g->mode == GHOST_SCATTER) {
            g->mode_before_fright = g->mode;
            g->mode = GHOST_FRIGHTENED;
            g->dir = direction_opposite(g->dir);
        }
    }
}

static void update_frightened(Game *game, float dt) {
    if (!game->frightened_active) return;
    game->frightened_timer -= dt;
    if (game->frightened_timer <= 0.0f) {
        game->frightened_active = false;
        for (int i = 0; i < GHOST_COUNT; i++) {
            if (game->ghosts[i].mode == GHOST_FRIGHTENED)
                game->ghosts[i].mode = (game->global_mode == GLOBAL_CHASE) ? GHOST_CHASE : GHOST_SCATTER;
        }
    }
}

// --- Ghost house ---

static void update_ghost_house(Game *game) {
    for (int i = 1; i < GHOST_COUNT; i++) {
        Ghost *g = &game->ghosts[i];
        if (g->mode != GHOST_IN_HOUSE) continue;
        bool should_exit = false;
        if (game->use_global_dot_counter) {
            if (game->global_dot_counter >= global_dot_limits[i]) {
                should_exit = true;
                if (i == GHOST_CLYDE) game->use_global_dot_counter = false;
            }
        } else {
            if (g->personal_dot_limit < 0 || g->personal_dot_counter >= g->personal_dot_limit)
                should_exit = true;
        }
        if (should_exit) {
            g->mode = GHOST_EXITING;
            game->no_dot_timer = 0.0f;
            break;
        }
    }
}

static void ghost_house_timer_fallback(Game *game) {
    for (int i = 1; i < GHOST_COUNT; i++) {
        if (game->ghosts[i].mode == GHOST_IN_HOUSE) {
            game->ghosts[i].mode = GHOST_EXITING;
            game->no_dot_timer = 0.0f;
            break;
        }
    }
}

// --- Dot consumption ---

static void on_dot_eaten(Game *game) {
    game->dots_eaten_this_level++;
    game->no_dot_timer = 0.0f;
    if (game->use_global_dot_counter) {
        game->global_dot_counter++;
    } else {
        for (int i = 1; i < GHOST_COUNT; i++) {
            if (game->ghosts[i].mode == GHOST_IN_HOUSE) {
                game->ghosts[i].personal_dot_counter++;
                break;
            }
        }
    }
}

static void check_dot_consumption(Game *game) {
    PacMan *pm = &game->pacman;
    int tx = pm->tile_x, ty = pm->tile_y;
    if (tx < 0 || tx >= MAZE_COLS || ty < 0 || ty >= MAZE_ROWS) return;
    if (game->dot_eaten[ty][tx]) return;

    int tile = maze_layout[ty][tx];
    int tier = speed_tier(game->level);

    if (tile == TILE_DOT) {
        game->dot_eaten[ty][tx] = true;
        game->score += 10;
        game->dots_remaining--;
        pm->eat_pause_frames = DOT_PAUSE_FRAMES;
        pm->speed_pct = pacman_speed_eating[tier];
        on_dot_eaten(game);
        play_sound(game, game->dot_sound_toggle ? SND_DOT_B : SND_DOT_A);
        game->dot_sound_toggle = !game->dot_sound_toggle;
        particles_spawn(game->particles, MAZE_OFFSET_X + pm->px, MAZE_OFFSET_Y + pm->py, COLOR_DOT, 4);
    } else if (tile == TILE_POWER_PELLET) {
        game->dot_eaten[ty][tx] = true;
        game->score += 50;
        game->dots_remaining--;
        pm->eat_pause_frames = PELLET_PAUSE_FRAMES;
        on_dot_eaten(game);
        activate_frightened(game);
        play_sound(game, SND_POWER_PELLET);
        particles_spawn(game->particles, MAZE_OFFSET_X + pm->px, MAZE_OFFSET_Y + pm->py, COLOR_PELLET, 12);
    }

    if (game->score > game->high_score)
        game->high_score = game->score;

    if (!game->extra_life_given && game->score >= 10000) {
        game->extra_life_given = true;
        game->lives++;
        play_sound(game, SND_EXTRA_LIFE);
    }
}

// --- Collision ---

static void check_ghost_collision(Game *game) {
    PacMan *pm = &game->pacman;
    for (int i = 0; i < GHOST_COUNT; i++) {
        Ghost *g = &game->ghosts[i];
        if (g->mode == GHOST_IN_HOUSE || g->mode == GHOST_EXITING || g->mode == GHOST_EATEN)
            continue;
        if (pm->tile_x == g->tile_x && pm->tile_y == g->tile_y) {
            if (g->mode == GHOST_FRIGHTENED) {
                game->ghost_eat_combo++;
                int points = 200;
                for (int c = 1; c < game->ghost_eat_combo; c++) points *= 2;
                game->score += points;
                if (game->score > game->high_score) game->high_score = game->score;
                g->mode = GHOST_EATEN;
                play_sound(game, SND_GHOST_EATEN);
                game->ghost_eaten_pause = true;
                game->ghost_eaten_pause_timer = 1.0f;
                game->ghost_eaten_score_display = points;
                game->ghost_eaten_display_x = g->px;
                game->ghost_eaten_display_y = g->py;
                return;
            } else {
                game->state = STATE_DYING;
                game->state_timer = 1.5f;
                game->pacman.death_timer = 0.0f;
                game->pacman.death_frame = 0;
                play_sound(game, SND_DEATH);
                return;
            }
        }
    }
}

// --- Game Update ---

void game_update(Game *game) {
    float dt = GetFrameTime();

    switch (game->state) {
    case STATE_TITLE:
        if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE) ||
            (IsGamepadAvailable(0) && (IsGamepadButtonPressed(0, GAMEPAD_BUTTON_RIGHT_FACE_DOWN) ||
                                       IsGamepadButtonPressed(0, GAMEPAD_BUTTON_MIDDLE_RIGHT)))) {
            game->level = 1;
            game->score = 0;
            game->lives = 3;
            game->extra_life_given = false;
            game_reset_level(game);
            game->state = STATE_READY;
            game->state_timer = 2.0f;
            play_sound(game, SND_MENU_SELECT);
        }
        break;

    case STATE_READY:
        game->state_timer -= dt;
        if (game->state_timer <= 0.0f) {
            game->state = STATE_PLAYING;
            play_sound(game, SND_READY);
        }
        break;

    case STATE_PLAYING:
        if (IsKeyPressed(KEY_ESCAPE) || IsKeyPressed(KEY_P) ||
            (IsGamepadAvailable(0) && IsGamepadButtonPressed(0, GAMEPAD_BUTTON_MIDDLE_RIGHT))) {
            game->state = STATE_PAUSED;
            break;
        }
        if (game->ghost_eaten_pause) {
            game->ghost_eaten_pause_timer -= dt;
            if (game->ghost_eaten_pause_timer <= 0.0f) game->ghost_eaten_pause = false;
            break;
        }

        pacman_update(&game->pacman, game->level, game->frightened_active, dt);
        check_dot_consumption(game);

        game->pellet_flash_timer += dt;
        if (game->pellet_flash_timer >= 0.2f) {
            game->pellet_flash_timer -= 0.2f;
            game->pellet_visible = !game->pellet_visible;
        }

        update_scatter_chase(game, dt);
        update_frightened(game, dt);

        update_ghost_house(game);
        game->no_dot_timer += dt;
        if (game->no_dot_timer >= game->no_dot_timeout) {
            ghost_house_timer_fallback(game);
            game->no_dot_timer = 0.0f;
        }

        // Fruit
        if (!game->fruit_spawned_70 && game->dots_eaten_this_level >= 70) {
            game->fruit_spawned_70 = true;
            fruit_spawn(&game->fruit, game->level);
        }
        if (!game->fruit_spawned_170 && game->dots_eaten_this_level >= 170) {
            game->fruit_spawned_170 = true;
            fruit_spawn(&game->fruit, game->level);
        }
        fruit_update(&game->fruit, dt);

        if (game->fruit.active &&
            game->pacman.tile_x == FRUIT_TILE_X && game->pacman.tile_y == FRUIT_TILE_Y) {
            game->score += game->fruit.points;
            if (game->score > game->high_score) game->high_score = game->score;
            game->fruit.score_display = true;
            game->fruit.score_display_timer = FRUIT_SCORE_DISPLAY_TIME;
            game->fruit.score_display_value = game->fruit.points;
            game->fruit.active = false;
            play_sound(game, SND_FRUIT);
            particles_spawn(game->particles,
                MAZE_OFFSET_X + tile_center_px(FRUIT_TILE_X),
                MAZE_OFFSET_Y + tile_center_px(FRUIT_TILE_Y),
                (Color){255, 100, 100, 255}, 8);
        }

        particles_update(game->particles, dt);

        for (int i = 0; i < GHOST_COUNT; i++)
            ghost_update_movement(&game->ghosts[i], i, game->level, game->dots_remaining,
                                  game->global_mode, &game->pacman, game->ghosts, dt);

        check_ghost_collision(game);

        if (game->dots_remaining <= 0) {
            game->state = STATE_LEVEL_COMPLETE;
            game->state_timer = 2.0f;
        }
        break;

    case STATE_DYING:
        game->state_timer -= dt;
        game->pacman.death_timer += dt;
        game->pacman.death_frame = (int)(game->pacman.death_timer / (1.5f / 11.0f));
        if (game->pacman.death_frame > 10) game->pacman.death_frame = 10;
        if (game->state_timer <= 0.0f) {
            game->lives--;
            if (game->lives > 0) {
                game_reset_positions(game);
                game->state = STATE_READY;
                game->state_timer = 2.0f;
            } else {
                highscore_save(game);
                game->state = STATE_GAME_OVER;
                game->state_timer = 3.0f;
            }
        }
        break;

    case STATE_LEVEL_COMPLETE:
        game->state_timer -= dt;
        game->level_complete_flash_white = ((int)(game->state_timer * 4.0f)) % 2 == 0;
        if (game->state_timer <= 0.0f) {
            game->level++;
            game_reset_level(game);
            game->state = STATE_READY;
            game->state_timer = 2.0f;
        }
        break;

    case STATE_GAME_OVER:
        game->state_timer -= dt;
        if (game->state_timer <= 0.0f) { highscore_save(game); game->state = STATE_TITLE; }
        break;

    case STATE_PAUSED:
        if (IsKeyPressed(KEY_ESCAPE) || IsKeyPressed(KEY_P) ||
            (IsGamepadAvailable(0) && IsGamepadButtonPressed(0, GAMEPAD_BUTTON_MIDDLE_RIGHT)))
            game->state = STATE_PLAYING;
        break;
    }
}

// --- Drawing ---

static bool is_wall(int c, int r) {
    if (c < 0 || c >= MAZE_COLS || r < 0 || r >= MAZE_ROWS) return false;
    return maze_layout[r][c] == TILE_WALL;
}

static void draw_maze(Game *game) {
    float line_thick = 2.5f;
    Color wall_color = game->level_complete_flash_white ? COLOR_WALL_WHITE : COLOR_WALL;
    Color glow_color = game->level_complete_flash_white ? (Color){255,255,255,80} : COLOR_WALL_GLOW;

    for (int r = 0; r < MAZE_ROWS; r++) {
        for (int c = 0; c < MAZE_COLS; c++) {
            int t = maze_layout[r][c];
            float x = MAZE_OFFSET_X + c * TILE_SIZE;
            float y = MAZE_OFFSET_Y + r * TILE_SIZE;

            if (t == TILE_WALL) {
                DrawRectangle((int)x, (int)y, TILE_SIZE, TILE_SIZE, (Color){15, 15, 40, 255});
                float half = line_thick / 2.0f;
                if (!is_wall(c, r-1)) { DrawRectangle((int)x,(int)(y-half),TILE_SIZE,(int)line_thick,wall_color); DrawRectangle((int)x,(int)(y-half-2),TILE_SIZE,(int)(line_thick+4),glow_color); }
                if (!is_wall(c, r+1)) { DrawRectangle((int)x,(int)(y+TILE_SIZE-half),TILE_SIZE,(int)line_thick,wall_color); DrawRectangle((int)x,(int)(y+TILE_SIZE-half-2),TILE_SIZE,(int)(line_thick+4),glow_color); }
                if (!is_wall(c-1, r)) { DrawRectangle((int)(x-half),(int)y,(int)line_thick,TILE_SIZE,wall_color); DrawRectangle((int)(x-half-2),(int)y,(int)(line_thick+4),TILE_SIZE,glow_color); }
                if (!is_wall(c+1, r)) { DrawRectangle((int)(x+TILE_SIZE-half),(int)y,(int)line_thick,TILE_SIZE,wall_color); DrawRectangle((int)(x+TILE_SIZE-half-2),(int)y,(int)(line_thick+4),TILE_SIZE,glow_color); }
                if (!is_wall(c, r-1) && !is_wall(c-1, r)) DrawCircle((int)x,(int)y,line_thick+1,wall_color);
                if (!is_wall(c, r-1) && !is_wall(c+1, r)) DrawCircle((int)(x+TILE_SIZE),(int)y,line_thick+1,wall_color);
                if (!is_wall(c, r+1) && !is_wall(c-1, r)) DrawCircle((int)x,(int)(y+TILE_SIZE),line_thick+1,wall_color);
                if (!is_wall(c, r+1) && !is_wall(c+1, r)) DrawCircle((int)(x+TILE_SIZE),(int)(y+TILE_SIZE),line_thick+1,wall_color);
            } else if (t == TILE_GHOST_DOOR) {
                DrawRectangle((int)x, (int)(y + TILE_SIZE/2 - 2), TILE_SIZE, 4, COLOR_GHOST_DOOR);
            }
        }
    }
}

static void draw_dots(Game *game) {
    for (int r = 0; r < MAZE_ROWS; r++) {
        for (int c = 0; c < MAZE_COLS; c++) {
            if (game->dot_eaten[r][c]) continue;
            int t = maze_layout[r][c];
            float cx = MAZE_OFFSET_X + c * TILE_SIZE + TILE_SIZE / 2.0f;
            float cy = MAZE_OFFSET_Y + r * TILE_SIZE + TILE_SIZE / 2.0f;
            if (t == TILE_DOT)
                DrawCircle((int)cx, (int)cy, 3.0f, COLOR_DOT);
            else if (t == TILE_POWER_PELLET && game->pellet_visible) {
                DrawCircle((int)cx, (int)cy, 10.0f, (Color){255,255,255,40});
                DrawCircle((int)cx, (int)cy, 7.0f, (Color){255,255,255,80});
                DrawCircle((int)cx, (int)cy, 5.0f, COLOR_PELLET);
            }
        }
    }
}

static void draw_hud(Game *game) {
    char buf[32];
    snprintf(buf, sizeof(buf), "SCORE: %d", game->score);
    DrawText(buf, MAZE_OFFSET_X, MAZE_OFFSET_Y + 4, 20, COLOR_TEXT);

    snprintf(buf, sizeof(buf), "HIGH SCORE: %d", game->high_score);
    int w = MeasureText(buf, 20);
    DrawText(buf, WINDOW_WIDTH/2 - w/2, MAZE_OFFSET_Y + 4, 20, COLOR_TEXT);

    snprintf(buf, sizeof(buf), "LEVEL: %d", game->level);
    w = MeasureText(buf, 20);
    DrawText(buf, MAZE_OFFSET_X + MAZE_WIDTH - w, MAZE_OFFSET_Y + 4, 20, COLOR_TEXT);

    for (int i = 0; i < game->lives; i++)
        DrawCircleSector((Vector2){MAZE_OFFSET_X + 20 + i * 30, MAZE_OFFSET_Y + MAZE_HEIGHT - 30},
                         10.0f, 30.0f, 330.0f, 16, COLOR_PACMAN);
}

void game_draw(Game *game) {
    BeginDrawing();
    ClearBackground(COLOR_BG);

    switch (game->state) {
    case STATE_TITLE: {
        const char *title = "PAC-MAN";
        DrawText(title, WINDOW_WIDTH/2 - MeasureText(title,60)/2, WINDOW_HEIGHT/2 - 80, 60, COLOR_PACMAN);
        const char *prompt = "PRESS ENTER OR START";
        DrawText(prompt, WINDOW_WIDTH/2 - MeasureText(prompt,24)/2, WINDOW_HEIGHT/2 + 20, 24, COLOR_TEXT);
        char buf[32];
        snprintf(buf, sizeof(buf), "HIGH SCORE: %d", game->high_score);
        DrawText(buf, WINDOW_WIDTH/2 - MeasureText(buf,20)/2, WINDOW_HEIGHT/2 + 70, 20, COLOR_DOT);
        break;
    }
    case STATE_READY:
        draw_maze(game); draw_dots(game); fruit_draw(&game->fruit);
        ghost_draw_all(game->ghosts, game->frightened_timer);
        pacman_draw(&game->pacman, false); draw_hud(game);
        { const char *r = "READY!"; DrawText(r, WINDOW_WIDTH/2 - MeasureText(r,30)/2, MAZE_OFFSET_Y + 20*TILE_SIZE, 30, COLOR_READY); }
        break;

    case STATE_PLAYING:
        draw_maze(game); draw_dots(game); fruit_draw(&game->fruit);
        ghost_draw_all(game->ghosts, game->frightened_timer);
        pacman_draw(&game->pacman, false);
        particles_draw(game->particles); draw_hud(game);
        if (game->frightened_active)
            DrawRectangle(0,0,WINDOW_WIDTH,WINDOW_HEIGHT,
                         (Color){0,0,80,(unsigned char)(30+20*sinf(game->frightened_timer*4.0f))});
        if (game->ghost_eaten_pause) {
            char pts[16]; snprintf(pts, sizeof(pts), "%d", game->ghost_eaten_score_display);
            DrawText(pts, (int)(MAZE_OFFSET_X+game->ghost_eaten_display_x-MeasureText(pts,16)/2),
                     (int)(MAZE_OFFSET_Y+game->ghost_eaten_display_y-8), 16, (Color){0,255,255,255});
        }
        break;

    case STATE_DYING:
        draw_maze(game); draw_dots(game); fruit_draw(&game->fruit);
        pacman_draw(&game->pacman, true);
        particles_draw(game->particles); draw_hud(game);
        break;

    case STATE_LEVEL_COMPLETE:
        draw_maze(game); draw_hud(game);
        break;

    case STATE_GAME_OVER:
        draw_maze(game); draw_dots(game); draw_hud(game);
        { const char *go = "GAME OVER"; DrawText(go, WINDOW_WIDTH/2 - MeasureText(go,40)/2, MAZE_OFFSET_Y + 20*TILE_SIZE, 40, (Color){255,0,0,255}); }
        break;

    case STATE_PAUSED:
        draw_maze(game); draw_dots(game);
        ghost_draw_all(game->ghosts, game->frightened_timer);
        pacman_draw(&game->pacman, false); draw_hud(game);
        DrawRectangle(0,0,WINDOW_WIDTH,WINDOW_HEIGHT,(Color){0,0,0,128});
        { const char *p = "PAUSED"; DrawText(p, WINDOW_WIDTH/2 - MeasureText(p,50)/2, WINDOW_HEIGHT/2 - 25, 50, COLOR_TEXT); }
        break;
    }
    EndDrawing();
}

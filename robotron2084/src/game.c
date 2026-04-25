#include "game.h"
#include "raymath.h"

static bool circles_overlap(Vector2 a, float ar, Vector2 b, float br) {
    float dx = a.x - b.x;
    float dy = a.y - b.y;
    float radius = ar + br;
    return dx * dx + dy * dy <= radius * radius;
}

static int count_active_grunts(Game *game) {
    int count = 0;
    for (int i = 0; i < MAX_GRUNTS; i++) {
        if (game->grunts[i].active) {
            count++;
        }
    }
    return count;
}

static int count_active_humans(Game *game) {
    int count = 0;
    for (int i = 0; i < MAX_HUMANS; i++) {
        if (game->humans[i].active) {
            count++;
        }
    }
    return count;
}

static void add_float_text(Game *game, Vector2 position, int value, Color color) {
    for (int i = 0; i < MAX_FLOAT_TEXT; i++) {
        FloatText *text = &game->float_text[i];
        if (!text->active) {
            text->active = true;
            text->position = position;
            text->velocity = (Vector2){ 0.0f, -42.0f };
            text->lifetime = FLOAT_TEXT_LIFETIME;
            text->value = value;
            text->color = color;
            return;
        }
    }
}

static void add_score(Game *game, int value) {
    game->score += value;
    if (game->score > game->high_score) {
        game->high_score = game->score;
    }

    while (game->score >= game->next_extra_life_score) {
        game->lives++;
        game->next_extra_life_score += EXTRA_LIFE_SCORE;
        add_float_text(game, (Vector2){ game->player_position.x, game->player_position.y - 28.0f }, 0, GREEN);
    }
}

static void reset_player(Game *game) {
    game->player_position = (Vector2){ PLAYER_START_X, PLAYER_START_Y };
    game->player_fire_timer = 0.0f;
    game->player_invulnerable_timer = RESPAWN_INVULN_TIME;
    game->humans_rescued_this_wave = 0;
    memset(game->bullets, 0, sizeof(game->bullets));
}

static void clear_wave_entities(Game *game) {
    memset(game->bullets, 0, sizeof(game->bullets));
    memset(game->grunts, 0, sizeof(game->grunts));
    memset(game->humans, 0, sizeof(game->humans));
    memset(game->float_text, 0, sizeof(game->float_text));
}

static Vector2 random_grunt_spawn_position(void) {
    int side = GetRandomValue(0, 3);
    float margin = 42.0f;
    Vector2 position = { 0.0f, 0.0f };

    if (side == 0) {
        position.x = (float)GetRandomValue((int)margin, WINDOW_WIDTH - (int)margin);
        position.y = margin;
    } else if (side == 1) {
        position.x = (float)GetRandomValue((int)margin, WINDOW_WIDTH - (int)margin);
        position.y = WINDOW_HEIGHT - margin;
    } else if (side == 2) {
        position.x = margin;
        position.y = (float)GetRandomValue((int)margin, WINDOW_HEIGHT - (int)margin);
    } else {
        position.x = WINDOW_WIDTH - margin;
        position.y = (float)GetRandomValue((int)margin, WINDOW_HEIGHT - (int)margin);
    }

    return position;
}

static Vector2 random_human_spawn_position(void) {
    float margin = 80.0f;
    return (Vector2){
        (float)GetRandomValue((int)margin, WINDOW_WIDTH - (int)margin),
        (float)GetRandomValue((int)margin, WINDOW_HEIGHT - (int)margin)
    };
}

static Vector2 random_human_velocity(void) {
    Vector2 direction = {
        (float)GetRandomValue(-100, 100) / 100.0f,
        (float)GetRandomValue(-100, 100) / 100.0f
    };

    if (direction.x == 0.0f && direction.y == 0.0f) {
        direction.x = 1.0f;
    }

    direction = Vector2Normalize(direction);
    return Vector2Scale(direction, HUMAN_SPEED);
}

static void spawn_grunt(Game *game, Vector2 position) {
    for (int i = 0; i < MAX_GRUNTS; i++) {
        Grunt *grunt = &game->grunts[i];
        if (!grunt->active) {
            grunt->active = true;
            grunt->position = position;
            grunt->speed = GRUNT_SPEED + (float)(game->wave - 1) * 8.0f;
            grunt->radius = GRUNT_RADIUS;
            return;
        }
    }
}

static void spawn_human(Game *game, Vector2 position, int type) {
    for (int i = 0; i < MAX_HUMANS; i++) {
        Human *human = &game->humans[i];
        if (!human->active) {
            human->active = true;
            human->type = type;
            human->position = position;
            human->velocity = random_human_velocity();
            human->radius = HUMAN_RADIUS;
            human->retarget_timer = (float)GetRandomValue(80, 180) / 100.0f;
            return;
        }
    }
}

static void spawn_wave(Game *game) {
    clear_wave_entities(game);
    reset_player(game);

    int grunt_count = WAVE_START_GRUNTS + (game->wave - 1) * WAVE_GRUNT_STEP;
    if (grunt_count > MAX_GRUNTS) {
        grunt_count = MAX_GRUNTS;
    }

    for (int i = 0; i < grunt_count; i++) {
        spawn_grunt(game, random_grunt_spawn_position());
    }

    int human_count = WAVE_START_HUMANS + game->wave / 2;
    if (human_count > WAVE_HUMAN_MAX) {
        human_count = WAVE_HUMAN_MAX;
    }

    for (int i = 0; i < human_count; i++) {
        spawn_human(game, random_human_spawn_position(), i % 3);
    }
}

static void start_new_game(Game *game) {
    game->score = 0;
    game->lives = PLAYER_LIVES;
    game->wave = 1;
    game->next_extra_life_score = EXTRA_LIFE_SCORE;
    game->humans_rescued_this_wave = 0;
    game->mode = GAME_MODE_PLAYING;
    spawn_wave(game);
}

static Vector2 get_move_input(void) {
    Vector2 move = { 0.0f, 0.0f };

    if (IsKeyDown(KEY_A)) move.x -= 1.0f;
    if (IsKeyDown(KEY_D)) move.x += 1.0f;
    if (IsKeyDown(KEY_W)) move.y -= 1.0f;
    if (IsKeyDown(KEY_S)) move.y += 1.0f;

    if (move.x != 0.0f || move.y != 0.0f) {
        move = Vector2Normalize(move);
    }

    return move;
}

static Vector2 get_fire_input(void) {
    Vector2 aim = { 0.0f, 0.0f };

    if (IsKeyDown(KEY_LEFT)) aim.x -= 1.0f;
    if (IsKeyDown(KEY_RIGHT)) aim.x += 1.0f;
    if (IsKeyDown(KEY_UP)) aim.y -= 1.0f;
    if (IsKeyDown(KEY_DOWN)) aim.y += 1.0f;

    if (aim.x != 0.0f || aim.y != 0.0f) {
        aim = Vector2Normalize(aim);
    }

    return aim;
}

static void spawn_bullet(Game *game, Vector2 direction) {
    for (int i = 0; i < MAX_BULLETS; i++) {
        Bullet *bullet = &game->bullets[i];
        if (!bullet->active) {
            bullet->active = true;
            bullet->position = game->player_position;
            bullet->velocity = Vector2Scale(direction, BULLET_SPEED);
            bullet->lifetime = BULLET_LIFETIME;
            return;
        }
    }
}

static int next_human_rescue_score(Game *game) {
    int rescue_index = game->humans_rescued_this_wave + 1;
    if (rescue_index >= 5) {
        return 5000;
    }
    return rescue_index * 1000;
}

static void update_playing(Game *game, float dt) {
    if (game->player_invulnerable_timer > 0.0f) {
        game->player_invulnerable_timer -= dt;
    }

    Vector2 move = get_move_input();
    game->player_position.x += move.x * game->player_speed * dt;
    game->player_position.y += move.y * game->player_speed * dt;

    game->player_position.x = Clamp(game->player_position.x, game->player_radius, WINDOW_WIDTH - game->player_radius);
    game->player_position.y = Clamp(game->player_position.y, game->player_radius, WINDOW_HEIGHT - game->player_radius);

    if (game->player_fire_timer > 0.0f) {
        game->player_fire_timer -= dt;
    }

    Vector2 aim = get_fire_input();
    if ((aim.x != 0.0f || aim.y != 0.0f) && game->player_fire_timer <= 0.0f) {
        spawn_bullet(game, aim);
        game->player_fire_timer = BULLET_FIRE_RATE;
    }

    for (int i = 0; i < MAX_BULLETS; i++) {
        Bullet *bullet = &game->bullets[i];
        if (!bullet->active) {
            continue;
        }

        bullet->position.x += bullet->velocity.x * dt;
        bullet->position.y += bullet->velocity.y * dt;
        bullet->lifetime -= dt;

        if (bullet->lifetime <= 0.0f ||
            bullet->position.x < -BULLET_RADIUS ||
            bullet->position.x > WINDOW_WIDTH + BULLET_RADIUS ||
            bullet->position.y < -BULLET_RADIUS ||
            bullet->position.y > WINDOW_HEIGHT + BULLET_RADIUS) {
            bullet->active = false;
        }
    }

    for (int i = 0; i < MAX_HUMANS; i++) {
        Human *human = &game->humans[i];
        if (!human->active) {
            continue;
        }

        human->retarget_timer -= dt;
        if (human->retarget_timer <= 0.0f) {
            human->velocity = random_human_velocity();
            human->retarget_timer = (float)GetRandomValue(80, 180) / 100.0f;
        }

        human->position.x += human->velocity.x * dt;
        human->position.y += human->velocity.y * dt;

        if (human->position.x < human->radius || human->position.x > WINDOW_WIDTH - human->radius) {
            human->velocity.x *= -1.0f;
            human->position.x = Clamp(human->position.x, human->radius, WINDOW_WIDTH - human->radius);
        }

        if (human->position.y < human->radius || human->position.y > WINDOW_HEIGHT - human->radius) {
            human->velocity.y *= -1.0f;
            human->position.y = Clamp(human->position.y, human->radius, WINDOW_HEIGHT - human->radius);
        }
    }

    for (int i = 0; i < MAX_GRUNTS; i++) {
        Grunt *grunt = &game->grunts[i];
        if (!grunt->active) {
            continue;
        }

        Vector2 to_player = Vector2Subtract(game->player_position, grunt->position);
        if (to_player.x != 0.0f || to_player.y != 0.0f) {
            Vector2 direction = Vector2Normalize(to_player);
            grunt->position.x += direction.x * grunt->speed * dt;
            grunt->position.y += direction.y * grunt->speed * dt;
        }
    }

    for (int bullet_index = 0; bullet_index < MAX_BULLETS; bullet_index++) {
        Bullet *bullet = &game->bullets[bullet_index];
        if (!bullet->active) {
            continue;
        }

        for (int grunt_index = 0; grunt_index < MAX_GRUNTS; grunt_index++) {
            Grunt *grunt = &game->grunts[grunt_index];
            if (!grunt->active) {
                continue;
            }

            if (circles_overlap(bullet->position, BULLET_RADIUS, grunt->position, grunt->radius)) {
                bullet->active = false;
                grunt->active = false;
                add_score(game, GRUNT_SCORE);
                break;
            }
        }
    }

    for (int i = 0; i < MAX_HUMANS; i++) {
        Human *human = &game->humans[i];
        if (!human->active) {
            continue;
        }

        if (circles_overlap(game->player_position, game->player_radius, human->position, human->radius)) {
            int rescue_score = next_human_rescue_score(game);
            human->active = false;
            game->humans_rescued_this_wave++;
            add_score(game, rescue_score);
            add_float_text(game, human->position, rescue_score, GOLD);
        }
    }

    for (int i = 0; i < MAX_FLOAT_TEXT; i++) {
        FloatText *text = &game->float_text[i];
        if (!text->active) {
            continue;
        }

        text->position.x += text->velocity.x * dt;
        text->position.y += text->velocity.y * dt;
        text->lifetime -= dt;
        if (text->lifetime <= 0.0f) {
            text->active = false;
        }
    }

    if (count_active_grunts(game) == 0) {
        game->wave++;
        spawn_wave(game);
        return;
    }

    if (game->player_invulnerable_timer <= 0.0f) {
        for (int i = 0; i < MAX_GRUNTS; i++) {
            Grunt *grunt = &game->grunts[i];
            if (!grunt->active) {
                continue;
            }

            if (circles_overlap(game->player_position, game->player_radius, grunt->position, grunt->radius)) {
                game->lives--;
                if (game->lives <= 0) {
                    game->mode = GAME_MODE_GAME_OVER;
                    memset(game->bullets, 0, sizeof(game->bullets));
                } else {
                    reset_player(game);
                }
                return;
            }
        }
    }
}

void game_init(Game *game) {
    memset(game, 0, sizeof(*game));
    game->mode = GAME_MODE_TITLE;
    game->lives = PLAYER_LIVES;
    game->wave = 1;
    game->next_extra_life_score = EXTRA_LIFE_SCORE;
    game->player_speed = PLAYER_SPEED;
    game->player_radius = PLAYER_RADIUS;
    game->player_color = SKYBLUE;
    reset_player(game);
}

void game_update(Game *game) {
    float dt = GetFrameTime();

    if ((game->mode == GAME_MODE_TITLE || game->mode == GAME_MODE_GAME_OVER) && IsKeyPressed(KEY_ENTER)) {
        start_new_game(game);
    }

    if (game->mode == GAME_MODE_PLAYING && IsKeyPressed(KEY_P)) {
        game->mode = GAME_MODE_PAUSED;
    } else if (game->mode == GAME_MODE_PAUSED && IsKeyPressed(KEY_P)) {
        game->mode = GAME_MODE_PLAYING;
    }

    if (game->mode == GAME_MODE_PLAYING) {
        update_playing(game, dt);
    }
}

static void draw_arena_grid(void) {
    Color major = (Color){ 42, 54, 68, 120 };
    Color minor = (Color){ 30, 38, 50, 90 };

    for (int x = 0; x <= WINDOW_WIDTH; x += 40) {
        DrawLine(x, 0, x, WINDOW_HEIGHT, (x % 120 == 0) ? major : minor);
    }

    for (int y = 0; y <= WINDOW_HEIGHT; y += 40) {
        DrawLine(0, y, WINDOW_WIDTH, y, (y % 120 == 0) ? major : minor);
    }
}

static void draw_playfield(Game *game) {
    draw_arena_grid();

    for (int i = 0; i < MAX_HUMANS; i++) {
        Human human = game->humans[i];
        if (!human.active) {
            continue;
        }

        Color color = GOLD;
        if (human.type == 1) {
            color = ORANGE;
        } else if (human.type == 2) {
            color = LIME;
        }

        DrawCircleV(human.position, human.radius + 5.0f, Fade(color, 0.35f));
        DrawCircleV(human.position, human.radius, color);
        DrawCircleV((Vector2){ human.position.x, human.position.y - 3.0f }, 3.0f, RAYWHITE);
    }

    for (int i = 0; i < MAX_GRUNTS; i++) {
        Grunt grunt = game->grunts[i];
        if (!grunt.active) {
            continue;
        }

        DrawCircleV(grunt.position, grunt.radius + 5.0f, (Color){ 255, 62, 103, 65 });
        DrawCircleV(grunt.position, grunt.radius, (Color){ 230, 40, 72, 255 });
        DrawCircleV((Vector2){ grunt.position.x - 4.0f, grunt.position.y - 3.0f }, 3.0f, BLACK);
        DrawCircleV((Vector2){ grunt.position.x + 4.0f, grunt.position.y - 3.0f }, 3.0f, BLACK);
    }

    for (int i = 0; i < MAX_BULLETS; i++) {
        Bullet bullet = game->bullets[i];
        if (!bullet.active) {
            continue;
        }

        DrawCircleV(bullet.position, BULLET_RADIUS + 3.0f, (Color){ 77, 214, 255, 70 });
        DrawCircleV(bullet.position, BULLET_RADIUS, RAYWHITE);
    }

    bool blink_off = game->player_invulnerable_timer > 0.0f && ((int)(game->player_invulnerable_timer * 12.0f) % 2) == 0;
    if (!blink_off || game->mode != GAME_MODE_PLAYING) {
        DrawCircleV(game->player_position, game->player_radius + 6.0f, (Color){ 77, 214, 255, 80 });
        DrawCircleV(game->player_position, game->player_radius, game->player_color);
        DrawCircleV(game->player_position, 4.0f, RAYWHITE);
    }

    for (int i = 0; i < MAX_FLOAT_TEXT; i++) {
        FloatText text = game->float_text[i];
        if (!text.active) {
            continue;
        }

        float alpha = Clamp(text.lifetime / FLOAT_TEXT_LIFETIME, 0.0f, 1.0f);
        const char *label = text.value > 0 ? TextFormat("+%d", text.value) : "EXTRA LIFE";
        DrawText(label, (int)text.position.x - MeasureText(label, 18) / 2, (int)text.position.y, 18, Fade(text.color, alpha));
    }
}

void game_draw(Game *game) {
    BeginDrawing();
    ClearBackground((Color){ 10, 12, 18, 255 });

    draw_playfield(game);

    DrawText("ROBOTRON 2084 - CORE COMBAT LOOP", 24, 24, 24, RAYWHITE);
    DrawText(TextFormat("SCORE %06d   HIGH %06d   WAVE %d   LIVES %d   GRUNTS %d   HUMANS %d",
        game->score,
        game->high_score,
        game->wave,
        game->lives,
        count_active_grunts(game),
        count_active_humans(game)), 24, 56, 18, LIGHTGRAY);
    DrawText(TextFormat("WASD move  |  Arrow keys fire  |  P pause  |  Enter start  |  Next rescue +%d",
        next_human_rescue_score(game)), 24, 82, 18, GRAY);

    if (game->mode == GAME_MODE_TITLE) {
        DrawRectangle(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, (Color){ 0, 0, 0, 150 });
        DrawText("ROBOTRON 2084", 396, 360, 56, RAYWHITE);
        DrawText("Press Enter", 520, 430, 28, SKYBLUE);
    } else if (game->mode == GAME_MODE_PAUSED) {
        DrawRectangle(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, (Color){ 0, 0, 0, 150 });
        DrawText("PAUSED", 514, 400, 48, RAYWHITE);
        DrawText("Press P to resume", 494, 460, 22, LIGHTGRAY);
    } else if (game->mode == GAME_MODE_GAME_OVER) {
        DrawRectangle(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, (Color){ 0, 0, 0, 170 });
        DrawText("GAME OVER", 460, 390, 48, RAYWHITE);
        DrawText(TextFormat("Final score: %d", game->score), 500, 445, 24, LIGHTGRAY);
        DrawText("Press Enter", 520, 485, 24, SKYBLUE);
    }

    EndDrawing();
}

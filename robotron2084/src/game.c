#include "game.h"
#include "raymath.h"

static void reset_player(Game *game) {
    game->player_position = (Vector2){ PLAYER_START_X, PLAYER_START_Y };
    game->player_fire_timer = 0.0f;
    memset(game->bullets, 0, sizeof(game->bullets));
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

static void update_playing(Game *game, float dt) {
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
}

void game_init(Game *game) {
    memset(game, 0, sizeof(*game));
    game->mode = GAME_MODE_TITLE;
    game->player_speed = PLAYER_SPEED;
    game->player_radius = PLAYER_RADIUS;
    game->player_color = SKYBLUE;
    reset_player(game);
}

void game_update(Game *game) {
    float dt = GetFrameTime();

    if (IsKeyPressed(KEY_ENTER)) {
        reset_player(game);
        game->mode = GAME_MODE_PLAYING;
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

    for (int i = 0; i < MAX_BULLETS; i++) {
        Bullet bullet = game->bullets[i];
        if (!bullet.active) {
            continue;
        }

        DrawCircleV(bullet.position, BULLET_RADIUS + 3.0f, (Color){ 77, 214, 255, 70 });
        DrawCircleV(bullet.position, BULLET_RADIUS, RAYWHITE);
    }

    DrawCircleV(game->player_position, game->player_radius + 6.0f, (Color){ 77, 214, 255, 80 });
    DrawCircleV(game->player_position, game->player_radius, game->player_color);
    DrawCircleV(game->player_position, 4.0f, RAYWHITE);
}

void game_draw(Game *game) {
    BeginDrawing();
    ClearBackground((Color){ 10, 12, 18, 255 });

    draw_playfield(game);

    DrawText("ROBOTRON 2084 - MOVEMENT PROTOTYPE", 24, 24, 24, RAYWHITE);
    DrawText("WASD move  |  Arrow keys fire  |  P pause  |  Enter start/reset", 24, 56, 18, LIGHTGRAY);

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
        DrawText("Press Enter", 520, 450, 24, SKYBLUE);
    }

    EndDrawing();
}

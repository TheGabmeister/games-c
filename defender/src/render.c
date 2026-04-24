#include "game.h"

#include <math.h>

static void draw_glow_circle(float x, float y, float radius, Color color) {
    Color soft = color;
    soft.a = 40;
    DrawCircleV((Vector2){ x, y }, radius * 1.9f, soft);
    soft.a = 72;
    DrawCircleV((Vector2){ x, y }, radius * 1.25f, soft);
    DrawCircleV((Vector2){ x, y }, radius, color);
}

static void draw_wrapped_line(const Game *game, Vector2 a, Vector2 b, float thick, Color color) {
    float ax = world_to_screen_x(game, a.x);
    float bx = ax + wrapped_delta(a.x, b.x);
    DrawLineEx((Vector2){ ax, a.y }, (Vector2){ bx, b.y }, thick, color);
    if (ax < 80.0f) DrawLineEx((Vector2){ ax + WORLD_WIDTH, a.y }, (Vector2){ bx + WORLD_WIDTH, b.y }, thick, color);
    if (ax > WINDOW_WIDTH - 80.0f) DrawLineEx((Vector2){ ax - WORLD_WIDTH, a.y }, (Vector2){ bx - WORLD_WIDTH, b.y }, thick, color);
}

static void draw_starfield(const Game *game) {
    for (int layer = 0; layer < 3; layer++) {
        Color color = { 70, 110, 150, (unsigned char)(70 + layer * 35) };
        float scale = 0.18f + (float)layer * 0.18f;
        int count = 70 + layer * 30;
        for (int i = 0; i < count; i++) {
            float seed = (float)(i * 977 + layer * 131);
            float wx = fmodf(seed * 37.7f + game->camera_x * scale + game->star_scroll * 80.0f, WORLD_WIDTH);
            float sx = fmodf(wx / WORLD_WIDTH * WINDOW_WIDTH + seed, WINDOW_WIDTH);
            float sy = 84.0f + fmodf(seed * 19.3f, 480.0f);
            DrawPixelV((Vector2){ sx, sy }, color);
        }
    }
}

static void draw_terrain(const Game *game) {
    for (int pass = 0; pass < 3; pass++) {
        Color color = COLOR_TERRAIN;
        float thick = 2.0f;
        float offset = 0.0f;
        if (pass == 0) {
            color.a = 42;
            thick = 10.0f;
            offset = 5.0f;
        } else if (pass == 1) {
            color.a = 90;
            thick = 5.0f;
            offset = 2.0f;
        }

        for (int i = 0; i < TERRAIN_SAMPLES - 1; i++) {
            float x0 = WORLD_WIDTH * (float)i / (float)(TERRAIN_SAMPLES - 1);
            float x1 = WORLD_WIDTH * (float)(i + 1) / (float)(TERRAIN_SAMPLES - 1);
            Vector2 a = { x0, game->terrain.height[i] + offset };
            Vector2 b = { x1, game->terrain.height[i + 1] + offset };
            if (near_camera(game, x0, 120.0f) || near_camera(game, x1, 120.0f)) {
                draw_wrapped_line(game, a, b, thick, color);
            }
        }
    }
}

static void draw_ship(const Game *game) {
    if (!game->player.alive) return;
    if (game->player.invuln_timer > 0.0f && ((int)(game->player.invuln_timer * 12.0f) % 2) == 0) return;

    float sx = world_to_screen_x(game, game->player.x);
    float sy = game->player.y;
    float dir = (float)game->player.facing;
    Vector2 nose = { sx + dir * 24.0f, sy };
    Vector2 wing_a = { sx - dir * 18.0f, sy - 12.0f };
    Vector2 wing_b = { sx - dir * 13.0f, sy + 13.0f };
    Vector2 tail = { sx - dir * 28.0f, sy };

    Color glow = COLOR_PLAYER;
    glow.a = 50;
    DrawCircleV((Vector2){ sx, sy }, 29.0f, glow);
    DrawTriangle(nose, wing_a, wing_b, COLOR_PLAYER);
    DrawLineEx(tail, nose, 3.0f, WHITE);
    DrawLineEx(wing_a, wing_b, 2.0f, (Color){ 22, 80, 118, 255 });

    Color flame = { 255, 180, 64, 180 };
    DrawTriangle((Vector2){ sx - dir * 24.0f, sy - 6.0f }, (Vector2){ sx - dir * 24.0f, sy + 6.0f }, (Vector2){ sx - dir * (42.0f + randf(0.0f, 8.0f)), sy }, flame);
}

static void draw_enemy(const Game *game, const Enemy *enemy) {
    float sx = world_to_screen_x(game, enemy->x);
    if (sx < -80.0f || sx > WINDOW_WIDTH + 80.0f) return;

    Color color = enemy->kind == ENEMY_MUTANT ? COLOR_MUTANT : COLOR_LANDER;
    if (enemy->kind == ENEMY_BAITER) color = COLOR_BAITER;

    float pulse = 1.0f + sinf(enemy->phase * 5.0f) * 0.12f;
    draw_glow_circle(sx, enemy->y, 12.0f * pulse, color);
    DrawCircleLines((int)sx, (int)enemy->y, 18.0f * pulse, WHITE);
    DrawLineEx((Vector2){ sx - 20.0f, enemy->y }, (Vector2){ sx + 20.0f, enemy->y }, 2.0f, color);
    DrawLineEx((Vector2){ sx, enemy->y - 20.0f }, (Vector2){ sx, enemy->y + 20.0f }, 2.0f, color);

    if (enemy->carrying && enemy->target_humanoid >= 0) {
        Color beam = { 255, 245, 125, 80 };
        DrawLineEx((Vector2){ sx, enemy->y + 16.0f }, (Vector2){ sx, game->humanoids[enemy->target_humanoid].y }, 5.0f, beam);
    }
}

static void draw_humanoid(const Game *game, const Humanoid *h) {
    if (h->state == HUMANOID_LOST) return;
    float sx = world_to_screen_x(game, h->x);
    if (sx < -50.0f || sx > WINDOW_WIDTH + 50.0f) return;

    Color color = h->state == HUMANOID_FALLING ? COLOR_LASER : COLOR_HUMANOID;
    draw_glow_circle(sx, h->y, 5.0f, color);
    DrawLineEx((Vector2){ sx, h->y + 4.0f }, (Vector2){ sx, h->y + 13.0f }, 2.0f, color);
    DrawLineEx((Vector2){ sx - 6.0f, h->y + 9.0f }, (Vector2){ sx + 6.0f, h->y + 9.0f }, 2.0f, color);
    DrawLineEx((Vector2){ sx, h->y + 13.0f }, (Vector2){ sx - 5.0f, h->y + 20.0f }, 2.0f, color);
    DrawLineEx((Vector2){ sx, h->y + 13.0f }, (Vector2){ sx + 5.0f, h->y + 20.0f }, 2.0f, color);
}

static void draw_lasers_and_bullets(const Game *game) {
    for (int i = 0; i < MAX_LASERS; i++) {
        const Laser *l = &game->lasers[i];
        if (!l->active) continue;
        float sx = world_to_screen_x(game, l->x);
        Vector2 a = { sx - (float)l->facing * 8.0f, l->y };
        Vector2 b = { sx + (float)l->facing * LASER_LENGTH, l->y };
        Color glow = COLOR_LASER;
        glow.a = 70;
        DrawLineEx(a, b, 8.0f, glow);
        DrawLineEx(a, b, 3.0f, COLOR_LASER);
        DrawLineEx(a, b, 1.0f, WHITE);
    }

    for (int i = 0; i < MAX_BULLETS; i++) {
        const EnemyBullet *b = &game->bullets[i];
        if (!b->active) continue;
        float sx = world_to_screen_x(game, b->x);
        if (sx < -20.0f || sx > WINDOW_WIDTH + 20.0f) continue;
        draw_glow_circle(sx, b->y, 4.0f, COLOR_LASER);
    }
}

static void draw_particles(const Game *game) {
    for (int i = 0; i < MAX_PARTICLES; i++) {
        const Particle *p = &game->particles[i];
        if (!p->active) continue;
        float sx = world_to_screen_x(game, p->x);
        if (sx < -60.0f || sx > WINDOW_WIDTH + 60.0f) continue;
        float alpha = clampf(p->life / p->max_life, 0.0f, 1.0f);
        Color color = p->color;
        color.a = (unsigned char)((float)color.a * alpha);
        DrawCircleV((Vector2){ sx, p->y }, p->radius * (0.5f + alpha), color);
    }
}

static void draw_radar_marker(float world_x, float y, Color color) {
    float rx = RADAR_X + (world_x / WORLD_WIDTH) * RADAR_WIDTH;
    float ry = RADAR_Y + clampf((y - SKY_TOP) / (TERRAIN_BASE_Y - SKY_TOP), 0.0f, 1.0f) * RADAR_HEIGHT;
    DrawRectangle((int)rx - 2, (int)ry - 2, 4, 4, color);
}

static void draw_hud(const Game *game) {
    DrawText(TextFormat("SCORE %06d", game->score), 24, 20, 22, COLOR_HUD);
    DrawText(TextFormat("HIGH %06d", game->high_score), 24, 48, 18, (Color){ 130, 175, 214, 255 });
    DrawText(TextFormat("LIVES %d", game->lives), 1035, 20, 22, COLOR_HUD);
    DrawText(TextFormat("BOMBS %d", game->smart_bombs), 1035, 48, 18, (Color){ 255, 199, 91, 255 });
    DrawText(TextFormat("WAVE %d", game->wave), 1155, 48, 18, COLOR_HUD);

    DrawRectangleLines(RADAR_X, RADAR_Y, RADAR_WIDTH, RADAR_HEIGHT, (Color){ 78, 148, 190, 150 });
    DrawRectangle(RADAR_X, RADAR_Y + RADAR_HEIGHT - 7, RADAR_WIDTH, 1, (Color){ 65, 255, 165, 110 });
    draw_radar_marker(game->player.x, game->player.y, COLOR_PLAYER);

    for (int i = 0; i < MAX_ENEMIES; i++) {
        const Enemy *e = &game->enemies[i];
        if (e->active) draw_radar_marker(e->x, e->y, e->kind == ENEMY_LANDER ? COLOR_LANDER : COLOR_MUTANT);
    }

    for (int i = 0; i < MAX_HUMANOIDS; i++) {
        const Humanoid *h = &game->humanoids[i];
        if (h->state != HUMANOID_LOST) draw_radar_marker(h->x, h->y, COLOR_HUMANOID);
    }
}

static void draw_center_text(const char *title, const char *subtitle, const char *hint) {
    int title_size = 72;
    int title_width = MeasureText(title, title_size);
    DrawText(title, WINDOW_WIDTH / 2 - title_width / 2, 230, title_size, (Color){ 96, 232, 255, 255 });

    int sub_size = 24;
    int sub_width = MeasureText(subtitle, sub_size);
    DrawText(subtitle, WINDOW_WIDTH / 2 - sub_width / 2, 318, sub_size, COLOR_HUD);

    int hint_size = 22;
    int hint_width = MeasureText(hint, hint_size);
    DrawText(hint, WINDOW_WIDTH / 2 - hint_width / 2, 430, hint_size, (Color){ 255, 212, 99, 255 });
}

void render_game(Game *game) {
    float shake_x = 0.0f;
    float shake_y = 0.0f;
    if (game->screen_shake > 0.0f) {
        float amount = game->screen_shake * 14.0f;
        shake_x = randf(-amount, amount);
        shake_y = randf(-amount, amount);
    }

    BeginDrawing();
    ClearBackground(COLOR_BG);

    BeginMode2D((Camera2D){ .offset = { shake_x, shake_y }, .target = { 0.0f, 0.0f }, .rotation = 0.0f, .zoom = 1.0f });
    draw_starfield(game);

    for (int y = 120; y < 620; y += 80) {
        DrawLine(0, y, WINDOW_WIDTH, y, COLOR_GRID);
    }

    draw_terrain(game);
    draw_lasers_and_bullets(game);

    for (int i = 0; i < MAX_HUMANOIDS; i++) draw_humanoid(game, &game->humanoids[i]);
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (game->enemies[i].active) draw_enemy(game, &game->enemies[i]);
    }

    draw_ship(game);
    draw_particles(game);
    EndMode2D();

    draw_hud(game);

    if (game->state == STATE_TITLE) {
        draw_center_text("DEFENDER", "MODERN RAYLIB ARCADE", "PRESS ENTER");
        DrawText("ARROWS/WASD MOVE  SPACE FIRE  SHIFT REVERSE  B BOMB  H HYPERSPACE", 285, 520, 18, (Color){ 150, 190, 225, 255 });
    } else if (game->state == STATE_READY) {
        const char *text = TextFormat("WAVE %d", game->wave);
        DrawText(text, WINDOW_WIDTH / 2 - MeasureText(text, 48) / 2, 320, 48, COLOR_HUD);
    } else if (game->state == STATE_WAVE_COMPLETE) {
        DrawText("WAVE CLEAR", WINDOW_WIDTH / 2 - MeasureText("WAVE CLEAR", 48) / 2, 320, 48, (Color){ 95, 255, 171, 255 });
    } else if (game->state == STATE_GAME_OVER) {
        draw_center_text("GAME OVER", TextFormat("FINAL SCORE %06d", game->score), "PRESS ENTER TO RESTART");
    }

    EndDrawing();
}

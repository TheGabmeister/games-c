#include "world_internal.h"
#include "textures.h"
#include "raymath.h"
#include <math.h>

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

static void draw_texture_centered(Game *game, TextureID id, Vector2 position, float size, Color tint) {
    if (!texture_is_ready(game, id)) {
        return;
    }

    Texture2D texture = game->textures[id];
    Rectangle source = { 0.0f, 0.0f, (float)texture.width, (float)texture.height };
    Rectangle dest = { position.x, position.y, size, size };
    Vector2 origin = { size * 0.5f, size * 0.5f };
    DrawTexturePro(texture, source, dest, origin, 0.0f, tint);
}

void world_draw_playfield(Game *game) {
    draw_arena_grid();

    for (int i = 0; i < MAX_ELECTRODES; i++) {
        Electrode electrode = game->electrodes[i];
        if (!electrode.active) continue;

        float pulse = 0.5f + 0.5f * sinf(electrode.pulse);
        Color glow = (Color){ 255, 190, 42, (unsigned char)(70 + pulse * 70) };
        DrawCircleV(electrode.position, electrode.radius + 7.0f + pulse * 3.0f, glow);
        if (texture_is_ready(game, TEXTURE_ELECTRODE)) {
            draw_texture_centered(game, TEXTURE_ELECTRODE, electrode.position, 34.0f + pulse * 3.0f, WHITE);
        } else {
            DrawCircleLines((int)electrode.position.x, (int)electrode.position.y, electrode.radius + 5.0f, ORANGE);
            DrawLineEx((Vector2){ electrode.position.x - 11.0f, electrode.position.y },
                (Vector2){ electrode.position.x + 11.0f, electrode.position.y }, 4.0f, YELLOW);
            DrawLineEx((Vector2){ electrode.position.x, electrode.position.y - 11.0f },
                (Vector2){ electrode.position.x, electrode.position.y + 11.0f }, 4.0f, YELLOW);
        }
    }

    for (int i = 0; i < MAX_HUMANS; i++) {
        Human human = game->humans[i];
        if (!human.active) continue;

        Color color = GOLD;
        TextureID texture_id = TEXTURE_HUMAN_MOMMY;
        if (human.type == 1) {
            color = ORANGE;
            texture_id = TEXTURE_HUMAN_DADDY;
        } else if (human.type == 2) {
            color = LIME;
            texture_id = TEXTURE_HUMAN_MIKEY;
        }

        DrawCircleV(human.position, human.radius + 5.0f, Fade(color, 0.35f));
        if (texture_is_ready(game, texture_id)) {
            draw_texture_centered(game, texture_id, human.position, 28.0f, WHITE);
        } else {
            DrawCircleV(human.position, human.radius, color);
            DrawCircleV((Vector2){ human.position.x, human.position.y - 3.0f }, 3.0f, RAYWHITE);
        }
    }

    for (int i = 0; i < MAX_HULKS; i++) {
        Hulk hulk = game->hulks[i];
        if (!hulk.active) continue;

        DrawCircleV(hulk.position, hulk.radius + 6.0f, (Color){ 60, 255, 120, 70 });
        if (texture_is_ready(game, TEXTURE_HULK)) {
            draw_texture_centered(game, TEXTURE_HULK, hulk.position, 52.0f, WHITE);
        } else {
            DrawCircleV(hulk.position, hulk.radius, (Color){ 38, 176, 82, 255 });
            DrawRectangle((int)(hulk.position.x - 12.0f), (int)(hulk.position.y - 8.0f), 24, 10, DARKGREEN);
        }
        if (hulk.stun_timer > 0.0f) {
            DrawCircleLines((int)hulk.position.x, (int)hulk.position.y, hulk.radius + 10.0f, RAYWHITE);
        }
    }

    for (int i = 0; i < MAX_SPHEROIDS; i++) {
        Spheroid spheroid = game->spheroids[i];
        if (!spheroid.active) continue;

        float pulse = 0.5f + 0.5f * sinf(GetTime() * 5.0f + (float)i);
        DrawCircleV(spheroid.position, spheroid.radius + 8.0f + pulse * 2.0f, (Color){ 70, 115, 255, 65 });
        if (texture_is_ready(game, TEXTURE_SPHEROID)) {
            draw_texture_centered(game, TEXTURE_SPHEROID, spheroid.position, 40.0f, WHITE);
        } else {
            DrawCircleLines((int)spheroid.position.x, (int)spheroid.position.y, spheroid.radius + 3.0f, BLUE);
            DrawCircleV(spheroid.position, spheroid.radius, (Color){ 64, 95, 240, 255 });
            DrawCircleV(spheroid.position, spheroid.radius * 0.45f, BLACK);
        }
    }

    for (int i = 0; i < MAX_ENFORCERS; i++) {
        Enforcer enforcer = game->enforcers[i];
        if (!enforcer.active) continue;

        DrawCircleV(enforcer.position, enforcer.radius + 7.0f, (Color){ 68, 200, 255, 60 });
        if (texture_is_ready(game, TEXTURE_ENFORCER)) {
            draw_texture_centered(game, TEXTURE_ENFORCER, enforcer.position, 36.0f, WHITE);
        } else {
            DrawPoly(enforcer.position, 3, enforcer.radius + 4.0f, 90.0f, SKYBLUE);
            DrawPoly(enforcer.position, 3, enforcer.radius - 2.0f, 90.0f, DARKBLUE);
            DrawCircleV(enforcer.position, 4.0f, RAYWHITE);
        }
    }

    for (int i = 0; i < MAX_QUARKS; i++) {
        Quark quark = game->quarks[i];
        if (!quark.active) continue;

        float pulse = 0.5f + 0.5f * sinf(GetTime() * 4.0f + (float)i);
        DrawCircleV(quark.position, quark.radius + 8.0f + pulse * 2.0f, (Color){ 245, 245, 245, 60 });
        if (texture_is_ready(game, TEXTURE_QUARK)) {
            draw_texture_centered(game, TEXTURE_QUARK, quark.position, 40.0f, WHITE);
        } else {
            DrawRectanglePro((Rectangle){ quark.position.x, quark.position.y, 30.0f, 30.0f },
                (Vector2){ 15.0f, 15.0f }, 45.0f, RAYWHITE);
            DrawRectanglePro((Rectangle){ quark.position.x, quark.position.y, 16.0f, 16.0f },
                (Vector2){ 8.0f, 8.0f }, 45.0f, BLACK);
        }
    }

    for (int i = 0; i < MAX_TANKS; i++) {
        Tank tank = game->tanks[i];
        if (!tank.active) continue;

        DrawCircleV(tank.position, tank.radius + 7.0f, (Color){ 255, 64, 42, 60 });
        if (texture_is_ready(game, TEXTURE_TANK)) {
            draw_texture_centered(game, TEXTURE_TANK, tank.position, 42.0f, WHITE);
        } else {
            DrawRectangle((int)(tank.position.x - 15.0f), (int)(tank.position.y - 12.0f), 30, 24, MAROON);
            DrawRectangle((int)(tank.position.x - 9.0f), (int)(tank.position.y - 7.0f), 18, 14, RED);
            Vector2 barrel = tank.velocity;
            if (barrel.x == 0.0f && barrel.y == 0.0f) {
                barrel = (Vector2){ 0.0f, -1.0f };
            } else {
                barrel = Vector2Normalize(barrel);
            }
            DrawLineEx(tank.position, Vector2Add(tank.position, Vector2Scale(barrel, 22.0f)), 4.0f, ORANGE);
        }
    }

    for (int i = 0; i < MAX_BRAINS; i++) {
        Brain brain = game->brains[i];
        if (!brain.active) continue;

        float pulse = 0.5f + 0.5f * sinf(GetTime() * 5.0f + brain.wobble);
        DrawCircleV(brain.position, brain.radius + 8.0f + pulse * 3.0f, (Color){ 170, 70, 255, 70 });
        if (texture_is_ready(game, TEXTURE_BRAIN)) {
            draw_texture_centered(game, TEXTURE_BRAIN, brain.position, 42.0f, WHITE);
        } else {
            DrawCircleV(brain.position, brain.radius, (Color){ 135, 62, 210, 255 });
            DrawCircleV((Vector2){ brain.position.x - 6.0f, brain.position.y - 3.0f }, 4.0f, BLACK);
            DrawCircleV((Vector2){ brain.position.x + 6.0f, brain.position.y - 3.0f }, 4.0f, BLACK);
            DrawCircleLines((int)brain.position.x, (int)brain.position.y, brain.radius + 3.0f, VIOLET);
        }
    }

    for (int i = 0; i < MAX_PROGS; i++) {
        Prog prog = game->progs[i];
        if (!prog.active) continue;

        DrawCircleV(prog.position, prog.radius + 6.0f, (Color){ 220, 68, 255, 65 });
        if (texture_is_ready(game, TEXTURE_PROG)) {
            draw_texture_centered(game, TEXTURE_PROG, prog.position, 32.0f, WHITE);
        } else {
            DrawPoly(prog.position, 4, prog.radius + 3.0f, 45.0f, PURPLE);
            DrawPoly(prog.position, 4, prog.radius - 2.0f, 45.0f, VIOLET);
            DrawCircleV(prog.position, 3.0f, RAYWHITE);
        }
    }

    for (int i = 0; i < MAX_GRUNTS; i++) {
        Grunt grunt = game->grunts[i];
        if (!grunt.active) continue;

        DrawCircleV(grunt.position, grunt.radius + 5.0f, (Color){ 255, 62, 103, 65 });
        if (texture_is_ready(game, TEXTURE_GRUNT)) {
            draw_texture_centered(game, TEXTURE_GRUNT, grunt.position, 34.0f, WHITE);
        } else {
            DrawCircleV(grunt.position, grunt.radius, (Color){ 230, 40, 72, 255 });
            DrawCircleV((Vector2){ grunt.position.x - 4.0f, grunt.position.y - 3.0f }, 3.0f, BLACK);
            DrawCircleV((Vector2){ grunt.position.x + 4.0f, grunt.position.y - 3.0f }, 3.0f, BLACK);
        }
    }

    world_draw_projectiles(game);
    world_draw_bullets(game);

    bool blink_off = game->player_invulnerable_timer > 0.0f && ((int)(game->player_invulnerable_timer * 12.0f) % 2) == 0;
    if (!blink_off || game->mode != GAME_MODE_PLAYING) {
        DrawCircleV(game->player_position, game->player_radius + 6.0f, (Color){ 77, 214, 255, 80 });
        if (texture_is_ready(game, TEXTURE_PLAYER)) {
            draw_texture_centered(game, TEXTURE_PLAYER, game->player_position, 36.0f, WHITE);
        } else {
            DrawCircleV(game->player_position, game->player_radius, game->player_color);
            DrawCircleV(game->player_position, 4.0f, RAYWHITE);
        }
    }

    world_draw_effects(game);
}

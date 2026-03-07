#include "raylib.h"
#include "game.h"
#include "maze.h"
#include "player.h"
#include "ghost.h"
#include "pellet.h"
#include "score.h"
#include "renderer.h"
#include "audio.h"

static GameState gs;

static void level_init(void) {
    maze_init(&gs);
    pellet_init(&gs);
    player_init(&gs);
    ghost_init_all(&gs);
}

static void game_start(void) {
    gs.score_state.high_score = gs.score_state.high_score; /* preserve */
    score_init(&gs, 3);
    level_init();
    gs.phase       = GAME_READY;
    gs.ready_timer = 3.0f;
}

static void next_level(void) {
    score_next_level(&gs);
    if (gs.score_state.level >= 256) {
        /* Kill screen: just show game over */
        gs.phase       = GAME_OVER;
        gs.phase_timer = 3.0f;
        return;
    }
    level_init();
    gs.phase       = GAME_READY;
    gs.ready_timer = 2.0f;
}

int main(void) {
    InitWindow(SCREEN_W, SCREEN_H, "PAC-MAN");
    SetTargetFPS(60);
    InitAudioDevice();
    audio_init();

    /* Start in menu but pre-initialize so the maze is visible behind it */
    score_init(&gs, 3);
    level_init();
    gs.phase = GAME_MENU;

    while (!WindowShouldClose()) {
        float dt = GetFrameTime();
        if (dt > 0.05f) dt = 0.05f; /* cap delta to avoid spiral of death */

        switch (gs.phase) {
            case GAME_MENU:
                if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE))
                    game_start();
                break;

            case GAME_READY:
                gs.ready_timer -= dt;
                if (gs.ready_timer <= 0.0f)
                    gs.phase = GAME_PLAYING;
                break;

            case GAME_PLAYING:
                player_input(&gs);
                ghost_update_schedule(&gs, dt);
                for (int i = 0; i < GHOST_COUNT; i++)
                    ghost_update(&gs, &gs.ghosts[i], dt);
                player_update(&gs, dt);
                pellet_update_frightened(&gs, dt);
                pellet_update_fruit(&gs, dt);
                audio_update(&gs);

                if (maze_dots_remaining(&gs) == 0) {
                    gs.phase       = GAME_LEVEL_COMPLETE;
                    gs.phase_timer = 2.5f;
                }
                break;

            case GAME_PAC_DYING:
                if (player_update_death(&gs, dt)) {
                    score_lose_life(&gs);
                    if (gs.score_state.lives <= 0) {
                        gs.phase       = GAME_OVER;
                        gs.phase_timer = 3.0f;
                    } else {
                        player_reset_position(&gs);
                        ghost_init_all(&gs);
                        gs.phase       = GAME_READY;
                        gs.ready_timer = 2.0f;
                    }
                }
                break;

            case GAME_LEVEL_COMPLETE:
                gs.phase_timer -= dt;
                if (gs.phase_timer <= 0.0f)
                    next_level();
                break;

            case GAME_OVER:
                gs.phase_timer -= dt;
                if (gs.phase_timer <= 0.0f)
                    gs.phase = GAME_MENU;
                break;
        }

        BeginDrawing();
        ClearBackground(BLACK);
        renderer_draw(&gs);
        EndDrawing();
    }

    audio_shutdown();
    CloseAudioDevice();
    CloseWindow();
    return 0;
}

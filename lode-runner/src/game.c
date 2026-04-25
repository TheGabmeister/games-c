#include "game.h"
#include "input.h"
#include "particles.h"
#include "sounds.h"

#include <string.h>

/* ——— Level loading ——— */

static void load_level(Game *game, int level_index) {
    game->level_index = level_index;
    game->level_loaded_from_file = world_load_level(
        &game->world, game->level_index,
        game->level_status, sizeof(game->level_status));
    player_spawn(&game->player, &game->world);
    guards_spawn_from_world(game->guards, &game->world);
    world_rebuild_pursuit(&game->world,
        game->player.actor.tile_r, game->player.actor.tile_c, game->pursuit);
    game->exit_revealed_prev = false;
    game->exit_reveal_timer = 0.0f;
    particles_init(&game->particles);
}

static void start_new_run(Game *game) {
    game->score = 0;
    game->lives = PLAYER_LIVES;
    game->screen = SCREEN_PLAY;
    game->screen_timer = 0.0f;
    game->victory = false;
    game->paused = false;
    load_level(game, 0);
}

static void start_level_clear(Game *game) {
    game->score += SCORE_LEVEL_CLEAR;
    game->lives++;
    game->screen = SCREEN_LEVEL_CLEAR;
    game->screen_timer = 1.5f;
    game->victory = game->level_index >= MAX_LEVELS - 1;
    sound_play(game->sounds, SOUND_LEVEL_CLEAR);
}

/* ——— Init / Shutdown ——— */

void game_init(Game *game) {
    memset(game, 0, sizeof(*game));
    game->screen = SCREEN_TITLE;
    game->lives = PLAYER_LIVES;
    particles_init(&game->particles);

    Image cave = GenImageCellular(256, 256, 16);
    ImageColorTint(&cave, (Color){ 22, 20, 32, 255 });
    game->bg_texture = LoadTextureFromImage(cave);
    UnloadImage(cave);
}

void game_shutdown(Game *game) {
    if (game->bg_texture.id != 0) {
        UnloadTexture(game->bg_texture);
        game->bg_texture.id = 0;
    }
}

/* ——— Play update ——— */

static void game_update_play(Game *game, float dt) {
    bool rebuild_pursuit = false;

    if (IsKeyPressed(KEY_PAGE_DOWN)) {
        load_level(game, (game->level_index + 1) % MAX_LEVELS);
    }
    if (IsKeyPressed(KEY_PAGE_UP)) {
        int next = game->level_index - 1;
        if (next < 0) next = MAX_LEVELS - 1;
        load_level(game, next);
    }

    WorldTickResult wr = world_update_holes(&game->world, dt,
        game->player.actor.tile_r, game->player.actor.tile_c);
    if (wr.refilled_hole) {
        sound_play(game->sounds, SOUND_REFILL);
        rebuild_pursuit = true;

        for (int i = 0; i < game->world.guard_count; i++) {
            Guard *guard = &game->guards[i];
            if (guard->active && guard->state != GSTATE_RESPAWN &&
                wr.refilled_tiles[guard->actor.tile_r][guard->actor.tile_c]) {
                guard_kill_in_refill(guard, &game->world);
                game->score += SCORE_GUARD_KILL;
                sound_play(game->sounds, SOUND_GUARD_DIE);
            }
        }
    }
    if (wr.trapped_target) {
        player_start_death(&game->player);
        sound_play(game->sounds, SOUND_PLAYER_DIE);
        game->shake_timer = SHAKE_DURATION;
        Vector2 pp = actor_pixel_position(&game->player.actor);
        particles_spawn_burst(&game->particles, pp.x, pp.y, 15, SKYBLUE, 120.0f, 0.5f);
    }

    PlayerTickResult pr = player_update(&game->player, &game->world, dt);
    if (pr.collected_gold) {
        game->score += SCORE_GOLD;
        sound_play(game->sounds, SOUND_COIN);
        if (game->world.all_gold_collected) {
            sound_play(game->sounds, SOUND_GOLD_COMPLETE);
        }
    }
    if (pr.dug_brick) {
        rebuild_pursuit = true;
        sound_play(game->sounds, SOUND_DIG);
        int dig_r = game->player.actor.tile_r + 1;
        int dig_c = game->player.actor.tile_c + game->player.actor.facing;
        float px = (float)PLAY_OFFSET_X + ((float)dig_c + 0.5f) * (float)TILE_SIZE;
        float py = (float)PLAY_OFFSET_Y + ((float)dig_r + 0.5f) * (float)TILE_SIZE;
        particles_spawn_burst(&game->particles, px, py, 12,
                              (Color){ 133, 73, 45, 255 }, 100.0f, 0.5f);
    }
    if (pr.committed_new_tile) {
        rebuild_pursuit = true;
    }
    if (pr.reached_exit) {
        start_level_clear(game);
        return;
    }

    if (game->world.all_gold_collected && !game->exit_revealed_prev) {
        game->exit_reveal_timer = EXIT_REVEAL_SEC;
        game->exit_revealed_prev = true;
    }

    if (rebuild_pursuit) {
        world_rebuild_pursuit(&game->world,
            game->player.actor.tile_r, game->player.actor.tile_c, game->pursuit);
    }

    for (int i = 0; i < game->world.guard_count; i++) {
        GuardTickResult gr = guard_update(&game->guards[i], i, game->guards,
            &game->world, &game->player, game->pursuit, dt);
        if (gr.fell_in_hole) {
            sound_play(game->sounds, SOUND_GUARD_FALL);
        }
    }

    if (game->player.state != PSTATE_DEAD) {
        for (int i = 0; i < game->world.guard_count; i++) {
            if (guard_can_catch_player(&game->guards[i], &game->player)) {
                player_start_death(&game->player);
                sound_play(game->sounds, SOUND_PLAYER_DIE);
                game->shake_timer = SHAKE_DURATION;
                Vector2 pp = actor_pixel_position(&game->player.actor);
                particles_spawn_burst(&game->particles, pp.x, pp.y, 15,
                                      SKYBLUE, 120.0f, 0.5f);
                break;
            }
        }
    }

    if (pr.died) {
        game->lives--;
        if (game->lives <= 0) {
            game->screen = SCREEN_GAME_OVER;
            game->screen_timer = 0.0f;
            return;
        }
        load_level(game, game->level_index);
    }
}

/* ——— Main update ——— */

void game_update(Game *game) {
    float dt = GetFrameTime();
    if (dt > 0.1f) dt = 0.1f;

    if (game->screen == SCREEN_TITLE) {
        if (input_confirm()) start_new_run(game);
        if (input_back()) game->quit = true;
        return;
    }

    if (game->screen == SCREEN_LEVEL_CLEAR) {
        if (game->victory) {
            if (input_confirm()) {
                game->screen = SCREEN_PLAY;
                game->victory = false;
                load_level(game, 0);
            }
            return;
        }
        game->screen_timer -= dt;
        if (game->screen_timer <= 0.0f || input_confirm()) {
            game->screen = SCREEN_PLAY;
            load_level(game, (game->level_index + 1) % MAX_LEVELS);
        }
        return;
    }

    if (game->screen == SCREEN_GAME_OVER) {
        if (input_confirm()) game->screen = SCREEN_TITLE;
        if (input_back()) game->quit = true;
        return;
    }

    if (input_back()) {
        game->paused = false;
        game->screen = SCREEN_TITLE;
        return;
    }

    if (input_pause()) game->paused = !game->paused;

    if (game->paused) return;

    particles_update(&game->particles, dt);
    if (game->shake_timer > 0.0f) game->shake_timer -= dt;
    if (game->exit_reveal_timer > 0.0f) game->exit_reveal_timer -= dt;

    if (input_restart() && game->player.state != PSTATE_DEAD) {
        game->lives--;
        if (game->lives <= 0) {
            game->screen = SCREEN_GAME_OVER;
            return;
        }
        load_level(game, game->level_index);
        return;
    }

    game_update_play(game, dt);
}

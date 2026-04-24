#include "game.h"
#include "sounds.h"

void game_init(Game *game) {
    new_game(game);
    sounds_load(game);
}

void game_update(Game *game) {
    float dt = GetFrameTime();
    if (dt > 0.033f) dt = 0.033f;

    if (IsKeyPressed(KEY_ENTER)) {
        if (game->state == STATE_TITLE || game->state == STATE_GAME_OVER) {
            bool had_sounds = game->sounds_loaded;
            Sound sounds[SOUND_COUNT];
            memcpy(sounds, game->sounds, sizeof(sounds));
            new_game(game);
            memcpy(game->sounds, sounds, sizeof(sounds));
            game->sounds_loaded = had_sounds;
            start_wave(game);
        }
    }

    game->state_timer -= dt;
    game->screen_shake = approach(game->screen_shake, 0.0f, dt * 1.6f);

    switch (game->state) {
        case STATE_TITLE:
        case STATE_GAME_OVER:
            game->camera_x = wrap_x(game->camera_x + dt * 48.0f);
            break;
        case STATE_READY:
            update_particles(game, dt);
            if (game->state_timer <= 0.0f) {
                game->state = STATE_PLAYING;
            }
            break;
        case STATE_PLAYING:
            update_player(game, dt);
            update_lasers(game, dt);
            update_bullets(game, dt);
            update_enemy_ai(game, dt);
            update_humanoids(game, dt);
            update_particles(game, dt);
            check_wave_complete(game);
            break;
        case STATE_DYING:
            update_lasers(game, dt);
            update_bullets(game, dt);
            update_enemy_ai(game, dt);
            update_humanoids(game, dt);
            update_particles(game, dt);
            if (game->state_timer <= 0.0f) {
                if (game->lives <= 0) {
                    game->state = STATE_GAME_OVER;
                    game->state_timer = 0.0f;
                } else {
                    reset_projectiles(game);
                    reset_player(game);
                    game->state = STATE_READY;
                    game->state_timer = 1.2f;
                }
            }
            break;
        case STATE_WAVE_COMPLETE:
            update_particles(game, dt);
            game->camera_x = wrap_x(game->camera_x + dt * 80.0f);
            if (game->state_timer <= 0.0f) {
                reset_player(game);
                start_wave(game);
            }
            break;
    }
}

void game_draw(Game *game) {
    render_game(game);
}

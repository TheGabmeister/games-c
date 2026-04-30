#include "title.h"
#include "game.h"
#include "input.h"
#include "raylib.h"
#include <stdio.h>

void title_refresh(TitleState *state) {
    for (int i = 0; i < SAVE_SLOT_COUNT; i++) {
        save_read_summary(i, &state->slots[i]);
    }
}

void title_update(Game *game) {
    TitleState *state = &game->title_state;
    if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W) ||
        IsGamepadButtonPressed(0, GAMEPAD_BUTTON_LEFT_FACE_UP)) {
        state->cursor--;
        if (state->cursor < 0) state->cursor = SAVE_SLOT_COUNT - 1;
    }
    if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S) ||
        IsGamepadButtonPressed(0, GAMEPAD_BUTTON_LEFT_FACE_DOWN)) {
        state->cursor++;
        if (state->cursor >= SAVE_SLOT_COUNT) state->cursor = 0;
    }
    if (input_confirm() || input_attack()) {
        int slot = state->cursor;
        if (state->slots[slot].exists) {
            if (!save_load_game(game, slot)) {
                save_start_new_game(game, slot);
            }
        } else {
            save_start_new_game(game, slot);
        }
    }
}

void title_draw(const TitleState *state) {
    DrawRectangle(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, (Color){ 10, 12, 18, 255 });
    DrawText("THE LEGEND OF ZELDA", 252, 148, 44, GOLD);
    DrawText("FILE SELECT", 392, 236, 30, WHITE);

    for (int i = 0; i < SAVE_SLOT_COUNT; i++) {
        int y = 320 + i * 116;
        Color c = (i == state->cursor) ? GOLD : WHITE;
        DrawRectangleLines(248, y - 22, 528, 76, c);
        DrawText(TextFormat("SLOT %d", i + 1), 280, y, 26, c);
        if (state->slots[i].exists) {
            const SaveSlotSummary *s = &state->slots[i];
            char where[32];
            if (s->in_dungeon)
                snprintf(where, sizeof(where), "DUNGEON %d", s->dungeon_id);
            else
                snprintf(where, sizeof(where), "%02d_%02d", s->screen_x, s->screen_y);
            DrawText(TextFormat("HP %d/%d  R %d  %s",
                                s->health, s->max_health, s->rupees, where),
                     420, y + 4, 20, LIGHTGRAY);
        } else {
            DrawText("NEW QUEST", 420, y + 4, 20, LIGHTGRAY);
        }
    }

    DrawText("UP/DOWN SELECT  ENTER START", 336, 732, 20, LIGHTGRAY);
}

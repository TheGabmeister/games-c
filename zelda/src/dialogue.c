#include "dialogue.h"
#include "game.h"
#include "input.h"
#include "sounds.h"
#include "raylib.h"
#include <string.h>

void dialogue_start(Game *game, const char *text) {
    strncpy(game->dialogue_state.text, text, CAVE_TEXT_MAX - 1);
    game->dialogue_state.text[CAVE_TEXT_MAX - 1] = '\0';
    game->state = STATE_DIALOGUE;
    sound_play(SOUND_NPC);
}

void dialogue_update(Game *game) {
    if (input_confirm() || input_attack() || input_back()) {
        game->state = STATE_PLAY;
    }
}

void dialogue_draw(const DialogueState *state) {
    int box_x = 96;
    int box_y = PLAY_AREA_Y + PLAY_AREA_HEIGHT - 168;
    int box_w = WINDOW_WIDTH - 192;
    int box_h = 120;

    DrawRectangle(box_x, box_y, box_w, box_h, (Color){ 0, 0, 0, 230 });
    DrawRectangleLines(box_x, box_y, box_w, box_h, WHITE);
    DrawText(state->text, box_x + 28, box_y + 28, 24, WHITE);
    DrawText("ENTER", box_x + box_w - 104, box_y + box_h - 32, 18, LIGHTGRAY);
}

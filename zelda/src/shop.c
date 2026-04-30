#include "shop.h"
#include "game.h"
#include "input.h"
#include "items.h"
#include "sounds.h"
#include "raylib.h"
#include <string.h>

static bool item_is_permanent(ItemType item) {
    switch (item) {
        case ITEM_BOOMERANG:
        case ITEM_BOW:
        case ITEM_CANDLE:
        case ITEM_SHIELD:
        case ITEM_RAFT:
        case ITEM_LADDER:
        case ITEM_BRACELET:
        case ITEM_FOOD:
        case ITEM_LETTER:
        case ITEM_POTION:
            return true;
        default:
            return false;
    }
}

static void grant_shop_item(Game *game, ItemType item) {
    Inventory *inv = &game->player.inventory;
    switch (item) {
        case ITEM_BOMB:
            inv->bombs += 4;
            if (inv->bombs > inv->bomb_capacity) inv->bombs = inv->bomb_capacity;
            break;
        case ITEM_ARROW:
            inv->arrows += 10;
            if (inv->arrows > inv->arrow_capacity) inv->arrows = inv->arrow_capacity;
            break;
        case ITEM_KEY:
            inv->keys++;
            break;
        case ITEM_SHIELD:
            inv->shield_tier = 1;
            inv->items |= item_bit(item);
            break;
        case ITEM_POTION:
            inv->items |= item_bit(item);
            game->player.health = game->player.max_health;
            break;
        default:
            if (item > ITEM_NONE && item < ITEM_TYPE_COUNT)
                inv->items |= item_bit(item);
            break;
    }
}

void shop_open(Game *game, const CaveShopMeta *shop) {
    game->shop_state.shop = *shop;
    game->shop_state.cursor = 0;
    game->shop_state.message[0] = '\0';
    game->state = STATE_SHOP;
    sound_play(SOUND_NPC);
}

void shop_update(Game *game) {
    ShopState *state = &game->shop_state;
    Inventory *inv = &game->player.inventory;

    if (input_back()) {
        game->state = STATE_PLAY;
        return;
    }
    if (IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_A) ||
        IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W) ||
        IsGamepadButtonPressed(0, GAMEPAD_BUTTON_LEFT_FACE_LEFT) ||
        IsGamepadButtonPressed(0, GAMEPAD_BUTTON_LEFT_FACE_UP)) {
        state->cursor--;
        if (state->cursor < 0) state->cursor = state->shop.item_count - 1;
    }
    if (IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_D) ||
        IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S) ||
        IsGamepadButtonPressed(0, GAMEPAD_BUTTON_LEFT_FACE_RIGHT) ||
        IsGamepadButtonPressed(0, GAMEPAD_BUTTON_LEFT_FACE_DOWN)) {
        state->cursor++;
        if (state->cursor >= state->shop.item_count) state->cursor = 0;
    }
    if (!(input_confirm() || input_attack())) return;

    ShopItemMeta offer = state->shop.items[state->cursor];
    if (offer.item <= ITEM_NONE || offer.item >= ITEM_TYPE_COUNT) return;

    if (item_is_permanent(offer.item) && (inv->items & item_bit(offer.item))) {
        strncpy(state->message, "YOU HAVE IT.", CAVE_TEXT_MAX - 1);
        state->message[CAVE_TEXT_MAX - 1] = '\0';
        return;
    }
    if (inv->rupees < offer.price) {
        strncpy(state->message, "NOT ENOUGH RUPEES.", CAVE_TEXT_MAX - 1);
        state->message[CAVE_TEXT_MAX - 1] = '\0';
        return;
    }

    inv->rupees -= offer.price;
    grant_shop_item(game, offer.item);
    strncpy(state->message, "SOLD.", CAVE_TEXT_MAX - 1);
    state->message[CAVE_TEXT_MAX - 1] = '\0';
    sound_play(SOUND_SHOP);
}

void shop_draw(const ShopState *state, const Game *game) {
    int box_x = 96;
    int box_y = PLAY_AREA_Y + PLAY_AREA_HEIGHT - 200;
    int box_w = WINDOW_WIDTH - 192;
    int box_h = 152;

    DrawRectangle(box_x, box_y, box_w, box_h, (Color){ 0, 0, 0, 230 });
    DrawRectangleLines(box_x, box_y, box_w, box_h, WHITE);
    DrawText(TextFormat("RUPEES %d", game->player.inventory.rupees),
             box_x + 24, box_y + 20, 22, YELLOW);

    for (int i = 0; i < state->shop.item_count; i++) {
        ShopItemMeta offer = state->shop.items[i];
        int x = box_x + 40 + i * 240;
        int y = box_y + 64;
        Color c = (i == state->cursor) ? GOLD : WHITE;
        DrawText(item_display_name(offer.item), x, y, 20, c);
        DrawText(TextFormat("%d", offer.price), x, y + 28, 20, c);
    }

    if (state->message[0] != '\0') {
        DrawText(state->message, box_x + 24, box_y + box_h - 34, 20, LIGHTGRAY);
    } else {
        DrawText("ENTER BUY  ESC LEAVE", box_x + 24, box_y + box_h - 34, 18, LIGHTGRAY);
    }
}

#include "tilemap.h"
#include "textures.h"
#include <string.h>

const TileDef tile_defs[TILE_TYPE_COUNT] = {
    [TILE_FLOOR]     = { TILE_FLOOR,     0, true,  ITEM_NONE },
    [TILE_WALL]      = { TILE_WALL,      1, false, ITEM_NONE },
    [TILE_WATER]     = { TILE_WATER,     2, false, ITEM_NONE },
    [TILE_DOOR]      = { TILE_DOOR,      4, true,  ITEM_NONE },
    [TILE_PUSHBLOCK] = { TILE_PUSHBLOCK, 5, false, ITEM_NONE },
    [TILE_STAIRS]    = { TILE_STAIRS,    6, true,  ITEM_NONE },
};

static TileType char_to_tile(char c) {
    switch (c) {
        case 'W': return TILE_WALL;
        case '.': return TILE_FLOOR;
        case '~': return TILE_WATER;
        case 'D': return TILE_DOOR;
        case 'P': return TILE_PUSHBLOCK;
        case 'S': return TILE_STAIRS;
        default:  return TILE_FLOOR;
    }
}

bool screen_load(Screen *screen, const char *path) {
    char *text = LoadFileText(path);
    if (!text) return false;

    memset(screen, 0, sizeof(*screen));

    int row = 0;
    const char *p = text;

    while (*p && row < SCREEN_TILES_Y) {
        const char *line_start = p;

        while (*p && *p != '\n') p++;
        int line_len = (int)(p - line_start);
        if (line_len > 0 && line_start[line_len - 1] == '\r') line_len--;

        if (*p == '\n') p++;

        if (line_len == 0) continue;
        if (line_start[0] == '#') continue;

        bool has_colon = false;
        for (int i = 0; i < line_len; i++) {
            if (line_start[i] == ':') { has_colon = true; break; }
        }
        if (has_colon) continue;

        if (line_len < SCREEN_TILES_X) continue;

        for (int col = 0; col < SCREEN_TILES_X; col++) {
            screen->tiles[row][col] = (uint8_t)char_to_tile(line_start[col]);
        }
        row++;
    }

    UnloadFileText(text);
    return row == SCREEN_TILES_Y;
}

void screen_draw(const Screen *screen) {
    for (int row = 0; row < SCREEN_TILES_Y; row++) {
        for (int col = 0; col < SCREEN_TILES_X; col++) {
            TileType t = (TileType)screen->tiles[row][col];
            int sprite = tile_defs[t].sprite_index;
            Rectangle src = texture_frame_rect(4, sprite);
            Vector2 pos = { (float)(col * TILE_SIZE), (float)(PLAY_AREA_Y + row * TILE_SIZE) };
            DrawTextureRec(textures[TEX_TILES], src, pos, WHITE);
        }
    }
}

bool screen_tile_blocked(const Screen *screen, Rectangle hitbox) {
    float rel_y = hitbox.y - PLAY_AREA_Y;

    int col_min = (int)(hitbox.x / TILE_SIZE);
    int col_max = (int)((hitbox.x + hitbox.width - 1) / TILE_SIZE);
    int row_min = (int)(rel_y / TILE_SIZE);
    int row_max = (int)((rel_y + hitbox.height - 1) / TILE_SIZE);

    if (col_min < 0) col_min = 0;
    if (col_max >= SCREEN_TILES_X) col_max = SCREEN_TILES_X - 1;
    if (row_min < 0) row_min = 0;
    if (row_max >= SCREEN_TILES_Y) row_max = SCREEN_TILES_Y - 1;

    for (int r = row_min; r <= row_max; r++) {
        for (int c = col_min; c <= col_max; c++) {
            if (!tile_defs[screen->tiles[r][c]].passable) return true;
        }
    }
    return false;
}

const TileDef *screen_tile_at_pixel(const Screen *screen, int px, int py) {
    int col = px / TILE_SIZE;
    int row = (py - PLAY_AREA_Y) / TILE_SIZE;
    if (col < 0 || col >= SCREEN_TILES_X || row < 0 || row >= SCREEN_TILES_Y) {
        return &tile_defs[TILE_WALL];
    }
    return &tile_defs[screen->tiles[row][col]];
}

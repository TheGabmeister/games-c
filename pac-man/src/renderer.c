#include "renderer.h"
#include "maze.h"
#include "pellet.h"
#include <math.h>

/* ---- Color palette ----------------------------------------------- */
static const Color WALL_OUTER = { 0,   0,   180, 255 };
static const Color WALL_INNER = { 0,   0,   255, 255 };
static const Color DOT_COLOR  = { 255, 185, 151, 255 };
static const Color GHOST_DOOR_COLOR = { 255, 182, 255, 255 };

static const Color GHOST_COLORS[GHOST_COUNT] = {
    { 255,  0,   0, 255 },   /* Blinky - red    */
    { 255, 184, 222, 255 },  /* Pinky  - pink   */
    {  0, 255, 255, 255 },   /* Inky   - cyan   */
    { 255, 184,  81, 255 },  /* Clyde  - orange */
};

static const Color FRIGHT_COLOR = { 33,  33, 222, 255 };
static const Color FRIGHT_FLASH = { 255, 255, 255, 255 };

/* ---- Utility ----------------------------------------------------- */
/* Tile top-left pixel (without HUD offset for drawing) */
static inline int tile_px(int col) { return col * TILE_SIZE; }
static inline int tile_py(int row) { return HUD_HEIGHT + row * TILE_SIZE; }

/* ---- Maze -------------------------------------------------------- */
static void draw_maze(const GameState *gs) {
    double t = GetTime();
    bool pellet_visible = (int)(t / 0.35) % 2 == 0;

    for (int r = 0; r < MAZE_ROWS; r++) {
        for (int c = 0; c < MAZE_COLS; c++) {
            int px = tile_px(c);
            int py = tile_py(r);
            TileType tile = (TileType)gs->tiles[r][c];

            switch (tile) {
                case TILE_WALL:
                    DrawRectangle(px, py, TILE_SIZE, TILE_SIZE, WALL_OUTER);
                    DrawRectangle(px+2, py+2, TILE_SIZE-4, TILE_SIZE-4, WALL_INNER);
                    break;

                case TILE_DOT:
                    DrawCircle(px + TILE_SIZE/2, py + TILE_SIZE/2, 2.0f, DOT_COLOR);
                    break;

                case TILE_POWER_PELLET:
                    if (pellet_visible)
                        DrawCircle(px + TILE_SIZE/2, py + TILE_SIZE/2, 5.0f, DOT_COLOR);
                    break;

                case TILE_GHOST_DOOR:
                    DrawRectangle(px, py + TILE_SIZE/2 - 2, TILE_SIZE, 4, GHOST_DOOR_COLOR);
                    break;

                default: break;
            }
        }
    }
}

/* ---- Player ------------------------------------------------------ */
static float dir_angle(Direction d) {
    switch (d) {
        case DIR_RIGHT: return 0.0f;
        case DIR_DOWN:  return 90.0f;
        case DIR_LEFT:  return 180.0f;
        case DIR_UP:    return 270.0f;
        default:        return 0.0f;
    }
}

static void draw_player(const GameState *gs) {
    const Player *p = &gs->player;
    if (!p->alive && gs->phase != GAME_PAC_DYING) return;

    Vector2 center = { p->x, p->y };
    float radius = TILE_SIZE * 0.45f;

    if (gs->phase == GAME_PAC_DYING) {
        /* Death animation: wide opening wedge that shrinks */
        float angle = p->death_angle;
        if (angle <= 0.0f) return;
        float facing = dir_angle(DIR_UP); /* faces up during death */
        DrawCircleSector(center, radius, facing, facing + (360.0f - angle), 32, YELLOW);
        return;
    }

    float facing = dir_angle(p->dir);
    float gap = p->mouth_angle;
    /* Draw the filled sector (the non-mouth part) */
    if (gap < 1.0f) {
        DrawCircleV(center, radius, YELLOW);
    } else {
        DrawCircleSector(center, radius,
                         facing + gap,
                         facing + 360.0f - gap,
                         32, YELLOW);
    }
}

/* ---- Ghost ------------------------------------------------------- */
static void draw_ghost_eyes(Vector2 center, float r, Direction d) {
    float ex = r * 0.35f;
    float ey = r * 0.25f;
    Vector2 left  = { center.x - ex, center.y - ey };
    Vector2 right = { center.x + ex, center.y - ey };
    float eye_r   = r * 0.30f;
    float pupil_r = eye_r * 0.55f;

    DrawCircleV(left,  eye_r, WHITE);
    DrawCircleV(right, eye_r, WHITE);

    /* Pupils offset by direction */
    float pdx = 0.0f, pdy = 0.0f;
    switch (d) {
        case DIR_LEFT:  pdx = -eye_r * 0.4f; break;
        case DIR_RIGHT: pdx =  eye_r * 0.4f; break;
        case DIR_UP:    pdy = -eye_r * 0.4f; break;
        case DIR_DOWN:  pdy =  eye_r * 0.4f; break;
        default: break;
    }
    DrawCircleV((Vector2){ left.x  + pdx, left.y  + pdy }, pupil_r, DARKBLUE);
    DrawCircleV((Vector2){ right.x + pdx, right.y + pdy }, pupil_r, DARKBLUE);
}

static void draw_ghost_frightened(Vector2 center, float r, bool flash_visible) {
    Color body = flash_visible ? FRIGHT_COLOR : FRIGHT_FLASH;

    /* Upper dome */
    DrawCircleSector(center, r, 180.0f, 360.0f, 16, body);

    /* Lower rectangle */
    int rx = (int)(center.x - r);
    int ry = (int)(center.y);
    DrawRectangle(rx, ry, (int)(r * 2), (int)r, body);

    /* Wavy bottom — 4 scallops */
    float scallop_r = r / 4.0f;
    for (int i = 0; i < 4; i++) {
        float sx = (center.x - r) + scallop_r + i * scallop_r * 2.0f;
        DrawCircle((int)sx, (int)(center.y + r), (int)scallop_r, BLACK);
    }

    /* Simple frightened face: two white dot eyes + zigzag mouth */
    float ex = r * 0.30f;
    float ey = r * 0.20f;
    DrawCircleV((Vector2){ center.x - ex, center.y - ey }, r * 0.15f, WHITE);
    DrawCircleV((Vector2){ center.x + ex, center.y - ey }, r * 0.15f, WHITE);

    /* Zigzag mouth */
    float my = center.y + r * 0.35f;
    float mx = center.x - r * 0.45f;
    float seg = r * 0.30f;
    for (int i = 0; i < 3; i++) {
        float y1 = (i % 2 == 0) ? my        : my + r * 0.2f;
        float y2 = (i % 2 == 0) ? my + r * 0.2f : my;
        DrawLineEx((Vector2){ mx + i * seg,       y1 },
                   (Vector2){ mx + (i+1) * seg,   y2 },
                   1.5f, WHITE);
    }
}

static void draw_ghost(const Ghost *g) {
    Vector2 center = { g->x, g->y };
    float r = TILE_SIZE * 0.45f;

    if (g->mode == GSTATE_EATEN) {
        /* Only eyes */
        draw_ghost_eyes(center, r, g->dir);

        /* Eaten score popup */
        if (g->eaten_display_timer > 0.0f) {
            const char *txt = TextFormat("%d", g->eaten_score);
            DrawText(txt,
                     (int)(center.x) - MeasureText(txt, 12)/2,
                     (int)(center.y) - 6,
                     12, CYAN);
        }
        return;
    }

    if (g->mode == GSTATE_FRIGHTENED) {
        draw_ghost_frightened(center, r, g->flash_visible);
        return;
    }

    /* Normal ghost body */
    Color body = GHOST_COLORS[g->id];

    /* Upper dome */
    DrawCircleSector(center, r, 180.0f, 360.0f, 16, body);

    /* Lower rectangle */
    int rx = (int)(center.x - r);
    int ry = (int)(center.y);
    DrawRectangle(rx, ry, (int)(r * 2), (int)r, body);

    /* Wavy bottom — 4 scallops */
    float scallop_r = r / 4.0f;
    for (int i = 0; i < 4; i++) {
        float sx = (center.x - r) + scallop_r + i * scallop_r * 2.0f;
        DrawCircle((int)sx, (int)(center.y + r), (int)scallop_r, BLACK);
    }

    draw_ghost_eyes(center, r, g->dir);
}

/* ---- Fruit icon -------------------------------------------------- */
static void draw_fruit(const GameState *gs) {
    if (!gs->fruit.active) return;
    Vector2 pos = tile_to_pixel(FRUIT_SPAWN_COL, FRUIT_SPAWN_ROW);
    float r = TILE_SIZE * 0.40f;
    Color col;
    switch (gs->fruit.type) {
        case FRUIT_CHERRY:     col = RED;        break;
        case FRUIT_STRAWBERRY: col = RED;        break;
        case FRUIT_ORANGE:     col = ORANGE;     break;
        case FRUIT_APPLE:      col = RED;        break;
        case FRUIT_MELON:      col = GREEN;      break;
        case FRUIT_GALAXIAN:   col = BLUE;       break;
        case FRUIT_BELL:       col = GOLD;       break;
        case FRUIT_KEY:        col = GOLD;       break;
        default:               col = WHITE;      break;
    }
    DrawCircleV(pos, r, col);
    /* Score label */
    const char *lbl = TextFormat("%d", gs->fruit.score_value);
    DrawText(lbl, (int)pos.x - MeasureText(lbl, 8)/2, (int)pos.y - 4, 8, WHITE);
}

/* ---- HUD --------------------------------------------------------- */
static void draw_hud(const GameState *gs) {
    DrawRectangle(0, 0, SCREEN_W, HUD_HEIGHT, BLACK);
    /* Score */
    DrawText("SCORE", 8, 4, 10, WHITE);
    DrawText(TextFormat("%d", gs->score_state.score), 8, 16, 14, WHITE);
    /* High score */
    const char *hi = "HI-SCORE";
    int hw = MeasureText(hi, 10);
    DrawText(hi, SCREEN_W/2 - hw/2, 4, 10, WHITE);
    DrawText(TextFormat("%d", gs->score_state.high_score),
             SCREEN_W/2 - MeasureText(TextFormat("%d", gs->score_state.high_score), 14)/2,
             16, 14, WHITE);
    /* Level */
    const char *lvl = TextFormat("LVL %d", gs->score_state.level);
    DrawText(lvl, SCREEN_W - MeasureText(lvl, 12) - 8, 14, 12, WHITE);
}

/* ---- Lives bar --------------------------------------------------- */
static void draw_lives(const GameState *gs) {
    int y = HUD_HEIGHT + MAZE_ROWS * TILE_SIZE;
    DrawRectangle(0, y, SCREEN_W, LIVES_HEIGHT, BLACK);
    /* Draw (lives-1) small Pac-Man icons */
    int icons = gs->score_state.lives - 1;
    if (icons < 0) icons = 0;
    if (icons > 5) icons = 5;
    for (int i = 0; i < icons; i++) {
        Vector2 c = { 14.0f + i * 22.0f, (float)y + LIVES_HEIGHT * 0.5f };
        DrawCircleSector(c, 7.0f, 30.0f, 330.0f, 16, YELLOW);
    }
    /* Fruit icons earned (one per level, up to 7 shown) */
    int max_icons = gs->score_state.level - 1;
    if (max_icons > 7) max_icons = 7;
    for (int i = 0; i < max_icons; i++) {
        float fx = (float)SCREEN_W - 10.0f - i * 18.0f;
        float fy = (float)y + LIVES_HEIGHT * 0.5f;
        FruitType ft = pellet_fruit_for_level(gs->score_state.level - i);
        Color fc;
        switch (ft) {
            case FRUIT_CHERRY:     fc = RED;    break;
            case FRUIT_STRAWBERRY: fc = RED;    break;
            case FRUIT_ORANGE:     fc = ORANGE; break;
            case FRUIT_APPLE:      fc = RED;    break;
            case FRUIT_MELON:      fc = GREEN;  break;
            case FRUIT_GALAXIAN:   fc = BLUE;   break;
            case FRUIT_BELL:       fc = GOLD;   break;
            default:               fc = GOLD;   break;
        }
        DrawCircleV((Vector2){ fx, fy }, 6.0f, fc);
    }
}

/* ---- Screens ----------------------------------------------------- */
static void draw_screen_menu(void) {
    DrawRectangle(0, HUD_HEIGHT + 8*TILE_SIZE, SCREEN_W, 4*TILE_SIZE, BLACK);
    const char *title = "PAC-MAN";
    DrawText(title, SCREEN_W/2 - MeasureText(title, 30)/2,
             HUD_HEIGHT + 9*TILE_SIZE, 30, YELLOW);
    const char *sub = "PRESS ENTER TO START";
    DrawText(sub, SCREEN_W/2 - MeasureText(sub, 14)/2,
             HUD_HEIGHT + 11*TILE_SIZE, 14, WHITE);
    /* Blink */
    double t = GetTime();
    if ((int)(t / 0.5) % 2 == 0) {
        const char *cr = "INSERT COIN";
        DrawText(cr, SCREEN_W/2 - MeasureText(cr, 12)/2,
                 HUD_HEIGHT + 13*TILE_SIZE, 12, YELLOW);
    }
}

static void draw_screen_ready(void) {
    const char *msg = "READY!";
    DrawText(msg, SCREEN_W/2 - MeasureText(msg, 16)/2,
             HUD_HEIGHT + 17*TILE_SIZE + 4, 16, YELLOW);
}

static void draw_screen_game_over(void) {
    const char *msg = "GAME  OVER";
    DrawText(msg, SCREEN_W/2 - MeasureText(msg, 18)/2,
             HUD_HEIGHT + 17*TILE_SIZE + 2, 18, RED);
}

static void draw_screen_level_complete(void) {
    /* Flash the maze blue/white */
    double t = GetTime();
    if ((int)(t / 0.15) % 2 == 0) {
        DrawRectangle(0, HUD_HEIGHT, SCREEN_W, MAZE_ROWS * TILE_SIZE, WHITE);
    }
}

/* ---- Master draw ------------------------------------------------- */
void renderer_draw(const GameState *gs) {
    draw_hud(gs);
    draw_maze(gs);

    if (gs->phase == GAME_LEVEL_COMPLETE) {
        draw_screen_level_complete();
    } else {
        /* Ghosts */
        for (int i = 0; i < GHOST_COUNT; i++)
            draw_ghost(&gs->ghosts[i]);

        /* Fruit */
        draw_fruit(gs);

        /* Player */
        draw_player(gs);
    }

    draw_lives(gs);

    /* Overlay screens */
    switch (gs->phase) {
        case GAME_MENU:         draw_screen_menu();      break;
        case GAME_READY:        draw_screen_ready();     break;
        case GAME_OVER:         draw_screen_game_over(); break;
        default: break;
    }
}

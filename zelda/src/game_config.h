#ifndef GAME_CONFIG_H
#define GAME_CONFIG_H

// Window / layout
#define WINDOW_WIDTH        1024
#define WINDOW_HEIGHT       960
#define TARGET_FPS          60

// Logical resolution (1:1 with window, no scaling)
#define LOGICAL_WIDTH       1024
#define LOGICAL_HEIGHT      960
#define RENDER_SCALE        1

// Tile size in pixels
#define TILE_SIZE           64

// Screen layout
#define HUD_HEIGHT          224
#define HUD_DIVIDER         32
#define PLAY_AREA_Y         (HUD_HEIGHT + HUD_DIVIDER)
#define PLAY_AREA_HEIGHT    (WINDOW_HEIGHT - HUD_HEIGHT - HUD_DIVIDER)
#define SCREEN_TILES_X      16
#define SCREEN_TILES_Y      11

// Overworld grid
#define OVERWORLD_COLS      16
#define OVERWORLD_ROWS      8
#define START_SCREEN_X      7
#define START_SCREEN_Y      4
#define SCREEN_PATH_MAX     64

// Transitions
#define SCROLL_FRAMES       30
#define FADE_OUT_FRAMES     15
#define FADE_IN_FRAMES      15

// Warps
#define MAX_WARPS_PER_SCREEN 4
#define WARP_DEST_MAX        32

// Player
#define PLAYER_LIVES        3
#define PLAYER_SPEED        256.0f
#define PLAYER_MAX_HEALTH   6
#define PLAYER_ANIM_FRAMES  8

#define GAMEPAD_DEADZONE    0.5f

// Combat
#define SWORD_ACTIVE_FRAMES     8
#define SWORD_COOLDOWN_FRAMES   12
#define SWORD_DAMAGE            1
#define KNOCKBACK_FRAMES        8
#define KNOCKBACK_DISTANCE      32
#define KNOCKBACK_SPEED         (KNOCKBACK_DISTANCE * TARGET_FPS / KNOCKBACK_FRAMES)
#define INVULN_FRAMES           60
#define MAX_ENEMIES_PER_SCREEN  16
#define LOW_HEALTH_THRESHOLD    2
#define LOW_HEALTH_BEEP_FRAMES  30

// Projectiles
#define MAX_PROJECTILES         32
#define ARROW_SPEED             512.0f
#define ARROW_DAMAGE            2
#define ARROW_MAX_RANGE         (SCREEN_TILES_X * TILE_SIZE)
#define BOOMERANG_SPEED         384.0f
#define BOOMERANG_MAX_RANGE     (6 * TILE_SIZE)
#define BOOMERANG_STUN_FRAMES   60
#define BOMB_FUSE_FRAMES        120
#define BOMB_BLAST_RADIUS       96.0f
#define BOMB_DAMAGE             4
#define BOMB_EXPLOSION_FRAMES   20
#define ENEMY_ROCK_SPEED        224.0f
#define ENEMY_SPEAR_SPEED       256.0f
#define ITEM_USE_FRAMES         12
#define DRAGON_BEAM_SPEED       300.0f
#define DRAGON_BEAM_DAMAGE      2

// Items (shared by inventory, dungeons, shops, HUD)
typedef enum ItemType {
    ITEM_NONE = 0,
    ITEM_BOOMERANG,
    ITEM_BOW,
    ITEM_BOMB,
    ITEM_CANDLE,
    ITEM_KEY,
    ITEM_MAP,
    ITEM_COMPASS,
    ITEM_HEART_CONTAINER,
    ITEM_FRAGMENT,
    ITEM_TYPE_COUNT
} ItemType;

// Direction (shared by player, camera, enemies, projectiles)
typedef enum Direction {
    DIR_S = 0,
    DIR_N,
    DIR_E,
    DIR_W,
    DIR_COUNT
} Direction;

// Sound identifiers
typedef enum SoundID {
    SOUND_COIN = 0,
    SOUND_SWORD_SWING,
    SOUND_SWORD_HIT,
    SOUND_ENEMY_DEATH,
    SOUND_PLAYER_DAMAGE,
    SOUND_LOW_HEALTH,
    SOUND_ARROW_FIRE,
    SOUND_BOMB_PLACE,
    SOUND_BOMB_EXPLODE,
    SOUND_SHIELD_BLOCK,
    SOUND_PICKUP_RUPEE,
    SOUND_PICKUP_HEART,
    SOUND_PICKUP_BOMB,
    SOUND_DOOR_OPEN,
    SOUND_KEY_USE,
    SOUND_SHUTTER_OPEN,
    SOUND_SECRET,
    SOUND_ITEM_GET,
    SOUND_BOSS_ROAR,
    SOUND_BOSS_DEFEAT,
    SOUND_COUNT
} SoundID;

static inline Direction opposite_dir(Direction d) {
    switch (d) {
        case DIR_N: return DIR_S;
        case DIR_S: return DIR_N;
        case DIR_E: return DIR_W;
        case DIR_W: return DIR_E;
        default: return d;
    }
}

#endif

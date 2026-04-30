#include "sounds.h"
#include "raylib.h"

static Sound sounds[SOUND_COUNT];
static bool sound_loaded[SOUND_COUNT];

static const char *sound_files[SOUND_COUNT] = {
    [SOUND_COIN]          = "assets/coin.wav",
    [SOUND_SWORD_SWING]   = "assets/sword_swing.wav",
    [SOUND_SWORD_HIT]     = "assets/sword_hit.wav",
    [SOUND_ENEMY_DEATH]   = "assets/enemy_death.wav",
    [SOUND_PLAYER_DAMAGE] = "assets/player_damage.wav",
    [SOUND_LOW_HEALTH]    = "assets/low_health.wav",
    [SOUND_ARROW_FIRE]    = "assets/arrow_fire.wav",
    [SOUND_BOMB_PLACE]    = "assets/bomb_place.wav",
    [SOUND_BOMB_EXPLODE]  = "assets/bomb_explode.wav",
    [SOUND_SHIELD_BLOCK]  = "assets/shield_block.wav",
    [SOUND_PICKUP_RUPEE]  = "assets/pickup_rupee.wav",
    [SOUND_PICKUP_HEART]  = "assets/pickup_heart.wav",
    [SOUND_PICKUP_BOMB]   = "assets/pickup_bomb.wav",
    [SOUND_DOOR_OPEN]     = "assets/door_open.wav",
    [SOUND_KEY_USE]       = "assets/key_use.wav",
    [SOUND_SHUTTER_OPEN]  = "assets/shutter_open.wav",
    [SOUND_SECRET]        = "assets/secret.wav",
    [SOUND_ITEM_GET]      = "assets/item_get.wav",
    [SOUND_BOSS_ROAR]     = "assets/boss_roar.wav",
    [SOUND_BOSS_DEFEAT]   = "assets/boss_defeat.wav",
    [SOUND_NPC]           = "assets/npc.wav",
    [SOUND_SHOP]          = "assets/shop.wav",
};

void sounds_load(void) {
    for (int i = 0; i < SOUND_COUNT; i++) {
        if (sound_files[i] && FileExists(sound_files[i])) {
            sounds[i] = LoadSound(sound_files[i]);
            sound_loaded[i] = IsSoundValid(sounds[i]);
        }
    }
}

void sounds_unload(void) {
    for (int i = 0; i < SOUND_COUNT; i++) {
        if (sound_loaded[i]) UnloadSound(sounds[i]);
    }
}

void sound_play(SoundID id) {
    if (sound_loaded[id]) {
        PlaySound(sounds[id]);
    }
}

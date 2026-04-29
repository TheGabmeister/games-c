#ifndef SOUNDS_H
#define SOUNDS_H

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

typedef struct Game Game;

void sounds_load(Game *game);
void sounds_unload(Game *game);
void sound_play(Game *game, SoundID id);

#endif

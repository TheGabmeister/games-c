#ifndef EVENT_BUS_H
#define EVENT_BUS_H

#include <stdbool.h>

typedef enum {
    EVENT_PLAY_SOUND,
    EVENT_PLAYER_DIED,
    EVENT_DRAGON_KILLED,
    EVENT_GATE_STATE_CHANGED,
    EVENT_GAME_WON,
    EVENT_COUNT
} EventType;

typedef struct {
    int   id;      /* cast to SoundId */
    float volume;
} PlaySoundData;

typedef struct {
    int dragon_type;
} DragonKilledData;

typedef struct {
    int  room_id;
    int  direction;
    bool open;
} GateStateChangedData;

typedef void (*EventCallback)(const void *data);

void event_bus_init(void);
void event_bus_subscribe(EventType type, EventCallback cb);
void event_bus_publish(EventType type, const void *data);

#endif

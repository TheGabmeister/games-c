#ifndef EVENT_BUS_H
#define EVENT_BUS_H

typedef enum {
    EVENT_GOAL_SCORED,
    EVENT_PLAY_SOUND,
    EVENT_COUNT
} EventType;

typedef struct {
    int player;
} GoalScoredData;

typedef struct {
    int   id;      // cast to SoundName
    float volume;
} PlaySoundData;

typedef void (*EventCallback)(const void *data);

void event_bus_init(void);
void event_bus_subscribe(EventType type, EventCallback cb);
void event_bus_publish(EventType type, const void *data);

#endif

#ifndef EVENT_BUS_H
#define EVENT_BUS_H

typedef enum {
    EVENT_GOAL_SCORED,
    EVENT_COUNT
} EventType;

typedef struct {
    int player;
} GoalScoredData;

typedef void (*EventCallback)(const void *data);

void event_bus_init(void);
void event_bus_subscribe(EventType type, EventCallback cb);
void event_bus_publish(EventType type, const void *data);

#endif

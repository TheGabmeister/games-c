#include "event_bus.h"

#define MAX_SUBSCRIBERS 8

static struct {
    EventCallback callbacks[MAX_SUBSCRIBERS];
    int count;
} _subscribers[EVENT_COUNT];

void event_bus_init(void)
{
    for (int i = 0; i < EVENT_COUNT; i++)
        _subscribers[i].count = 0;
}

void event_bus_subscribe(EventType type, EventCallback cb)
{
    int *n = &_subscribers[type].count;
    if (*n < MAX_SUBSCRIBERS)
        _subscribers[type].callbacks[(*n)++] = cb;
}

void event_bus_publish(EventType type, const void *data)
{
    for (int i = 0; i < _subscribers[type].count; i++)
        _subscribers[type].callbacks[i](data);
}

#include "event_bus.h"

#include <SDL3/SDL.h>

#define MAX_SUBSCRIBERS 8

static struct {
    EventCallback callbacks[MAX_SUBSCRIBERS];
    int count;
} _subscribers[EVENT_COUNT];

static bool _is_valid_event_type(EventType type)
{
    return type >= 0 && type < EVENT_COUNT;
}

void event_bus_init(void)
{
    for (int i = 0; i < EVENT_COUNT; i++) {
        _subscribers[i].count = 0;
        for (int j = 0; j < MAX_SUBSCRIBERS; ++j)
            _subscribers[i].callbacks[j] = NULL;
    }
}

void event_bus_subscribe(EventType type, EventCallback cb)
{
    if (!_is_valid_event_type(type))
    {
        SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "event_bus_subscribe: invalid event type %d", (int)type);
        return;
    }

    if (!cb)
    {
        SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "event_bus_subscribe: null callback for event type %d", (int)type);
        return;
    }

    int *n = &_subscribers[type].count;
    if (*n < MAX_SUBSCRIBERS) {
        _subscribers[type].callbacks[(*n)++] = cb;
        return;
    }

    SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION,
                "event_bus_subscribe: subscriber limit reached for event type %d",
                (int)type);
}

void event_bus_publish(EventType type, const void *data)
{
    if (!_is_valid_event_type(type))
    {
        SDL_LogWarn(SDL_LOG_CATEGORY_APPLICATION, "event_bus_publish: invalid event type %d", (int)type);
        return;
    }

    for (int i = 0; i < _subscribers[type].count; i++) {
        EventCallback cb = _subscribers[type].callbacks[i];
        if (cb)
            cb(data);
    }
}

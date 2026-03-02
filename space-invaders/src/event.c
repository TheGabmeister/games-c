#include "event.h"

void event_queue_init(EventQueue *queue)
{
    if (!queue) return;

    queue->head = 0;
    queue->tail = 0;
    queue->count = 0;
    queue->dropped = 0;
}

void event_queue_clear(EventQueue *queue)
{
    if (!queue) return;

    queue->head = 0;
    queue->tail = 0;
    queue->count = 0;
}

bool event_queue_push(EventQueue *queue, const GameEvent *event)
{
    if (!queue || !event) return false;
    if (queue->count >= EVENT_QUEUE_CAPACITY) {
        queue->dropped++;
        return false;
    }

    queue->events[queue->tail] = *event;
    queue->tail = (queue->tail + 1) % EVENT_QUEUE_CAPACITY;
    queue->count++;
    return true;
}

bool event_queue_pop(EventQueue *queue, GameEvent *out_event)
{
    if (!queue || queue->count == 0) return false;

    if (out_event) {
        *out_event = queue->events[queue->head];
    }
    queue->head = (queue->head + 1) % EVENT_QUEUE_CAPACITY;
    queue->count--;
    return true;
}

void event_queue_dispatch(EventQueue *queue, event_handler_fn handler, void *ctx)
{
    GameEvent event;
    if (!queue || !handler) return;

    while (event_queue_pop(queue, &event)) {
        handler(&event, ctx);
    }
}

size_t event_queue_count(const EventQueue *queue)
{
    return queue ? queue->count : 0;
}

uint32_t event_queue_dropped(const EventQueue *queue)
{
    return queue ? queue->dropped : 0;
}

#ifndef EVENT_H
#define EVENT_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <flecs.h>

#ifndef EVENT_QUEUE_CAPACITY
#define EVENT_QUEUE_CAPACITY 256
#endif

typedef enum GameEventType {
    GAME_EVENT_NONE = 0,
    GAME_EVENT_ENEMY_KILLED,
    GAME_EVENT_SCORE_CHANGED,
    GAME_EVENT_PLAY_SOUND,
    GAME_EVENT_GAME_OVER
} GameEventType;

typedef struct EnemyKilledEvent {
    ecs_entity_t enemy;
    ecs_entity_t source;
    int points;
    float x;
    float y;
} EnemyKilledEvent;

typedef struct ScoreChangedEvent {
    int old_score;
    int new_score;
} ScoreChangedEvent;

typedef struct PlaySoundEvent {
    const char *sound_id;
} PlaySoundEvent;

typedef struct GameOverEvent {
    bool victory;
} GameOverEvent;

typedef struct GameEvent {
    GameEventType type;
    union {
        EnemyKilledEvent enemy_killed;
        ScoreChangedEvent score_changed;
        PlaySoundEvent play_sound;
        GameOverEvent game_over;
    } data;
} GameEvent;

typedef struct EventQueue {
    GameEvent events[EVENT_QUEUE_CAPACITY];
    size_t head;
    size_t tail;
    size_t count;
    uint32_t dropped;
} EventQueue;

typedef void (*event_handler_fn)(const GameEvent *event, void *ctx);

void event_queue_init(EventQueue *queue);
void event_queue_clear(EventQueue *queue);
bool event_queue_push(EventQueue *queue, const GameEvent *event);
bool event_queue_pop(EventQueue *queue, GameEvent *out_event);
void event_queue_dispatch(EventQueue *queue, event_handler_fn handler, void *ctx);
size_t event_queue_count(const EventQueue *queue);
uint32_t event_queue_dropped(const EventQueue *queue);

/* Global event queue — push from any system, dispatched once per frame in game.c. */
extern EventQueue g_events;

#endif /* EVENT_H */

#ifndef GAME_STATE_H
#define GAME_STATE_H

/*
 * Game State / Scene System
 * =========================
 * Manages discrete game states (menu, gameplay, pause, etc.) with clean
 * transitions. Each state provides its own init/update/draw/cleanup logic.
 *
 * --- How to add a new state ---
 *
 * 1. Add an entry to the GameStateID enum below (e.g., STATE_SETTINGS).
 *
 * 2. Create a new file pair (e.g., state_settings.c/h) and implement the
 *    four callbacks:
 *
 *      void settings_init(void);          // Load assets, set up variables
 *      void settings_update(float dt);    // Game logic, input handling
 *      void settings_draw(void);          // Render the state
 *      void settings_cleanup(void);       // Free assets
 *
 *    Any callback can be NULL if the state doesn't need it.
 *
 * 3. Register the state in your initialization code:
 *
 *      game_state_register(STATE_SETTINGS, (GameState){
 *          .init    = settings_init,
 *          .update  = settings_update,
 *          .draw    = settings_draw,
 *          .cleanup = settings_cleanup,
 *      });
 *
 * 4. Switch to it from anywhere:
 *
 *      game_state_switch(STATE_SETTINGS);
 *
 *    This calls cleanup() on the current state, then init() on the new one.
 */

#define MAX_GAME_STATES 16

/* Add your state IDs here */
typedef enum {
    STATE_NONE = -1,
    STATE_MENU,
    STATE_GAMEPLAY,
    STATE_PAUSE,
    /* STATE_SETTINGS, */
    /* STATE_GAME_OVER, */
    STATE_COUNT
} GameStateID;

/* Each state provides up to four callbacks (any may be NULL) */
typedef struct {
    void (*init)(void);
    void (*update)(float dt);
    void (*draw)(void);
    void (*cleanup)(void);
} GameState;

/* Register a state's callbacks under the given ID */
void game_state_register(GameStateID id, GameState state);

/* Transition: calls cleanup() on the current state, init() on the new one */
void game_state_switch(GameStateID id);

/* Call these from your main loop — they forward to the active state */
void game_state_update(float dt);
void game_state_draw(void);

/* Clean up the current state (call before exiting the program) */
void game_state_shutdown(void);

/* Query the active state */
GameStateID game_state_current(void);

#endif /* GAME_STATE_H */

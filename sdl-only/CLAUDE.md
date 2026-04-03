# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Build Commands

```bash
# Configure (Windows / Visual Studio 2026)
cmake -S . -B build -G "Visual Studio 18 2026" -A x64 -Wno-dev

# Build
cmake --build build --config Release

# Executable
build/Release/sdl_only.exe
```

New `.c` files must be added to the `add_executable()` list in `CMakeLists.txt` manually. There is no automated test suite; validation is a successful build plus manual run.

## Architecture

Pure C17 project using SDL3 (vendored in `vendor/sdl/`). No sprites, textures, or audio — rendering uses SDL's built-in debug font (8x8 per character) and primitive drawing only.

### Module dependency graph

```
main.c → config, platform, input, game, scene
game.c → scene, scenes/*
scenes/* → platform, input, draw, scene, collectible, enemy, gamestate (+ other scenes for transitions)
collectible → platform, gamestate, draw
enemy → platform, gamestate, draw
draw → platform
input → platform (for types only)
platform → SDL3
config, gamestate, scene → nothing
```

### Modules

- **config** (`config.h`) — Game-wide constants: `WINDOW_WIDTH`, `WINDOW_HEIGHT`, `WINDOW_TITLE`.
- **platform** (`platform.h/c`) — Window lifecycle, timing (nanosecond delta via `SDL_GetTicksNS`), renderer access. Owns the SDL_Window/SDL_Renderer. Defines shared types: `rectangle` (SDL_FRect), `color` (SDL_Color), `vector2-4`, `camera`.
- **input** (`input.h/c`) — Action-based input with rebindable keys. Actions (`ACTION_UP`, `ACTION_DOWN`, etc.) map to up to `MAX_BINDINGS` scancodes each. Raw `is_key_down`/`is_key_pressed` and mouse functions also live here.
- **draw** (`draw.h/c`) — Rendering primitives: `draw_rect`, `draw_rect_filled`, `draw_line`, `draw_circle`, `draw_circle_filled`, `draw_text`. Color presets as macros (`COLOR_WHITE`, `COLOR_RED`, etc.). Text uses `SDL_RenderDebugText` (8x8 fixed font), scaled via `SDL_SetRenderScale`.
- **scene** (`scene.h/c`) — Scene manager. A `Scene` is a struct of 4 function pointers: `init`, `update`, `draw`, `cleanup`. `scene_set()` auto-cleans the previous scene before initing the new one.
- **gamestate** (`gamestate.h/c`) — Shared game state (`score`, `health`) accessible from any scene via `gamestate_get()`. Reset with `gamestate_reset()`.
- **collectible** (`collectible.h/c`) — Reusable collectible system. Three types: `COLLECTIBLE_COIN` (+10 score), `COLLECTIBLE_HEALTH` (+1 HP), `COLLECTIBLE_STAR` (+50 score). AABB collision with player rect. Up to `MAX_COLLECTIBLES` (64).
- **enemy** (`enemy.h/c`) — Reusable enemy system. Three types: `ENEMY_WANDERER` (random direction changes), `ENEMY_CHASER` (follows player), `ENEMY_BOUNCER` (bounces off walls). Damages player on collision with 1s cooldown. Up to `MAX_ENEMIES` (32).
- **game** (`game.c`) — Glue: sets the starting scene and handles shutdown.

### Scenes

Scene flow: `title_scene` → `play_scene` → `gameover_scene` (on death). Each scene is in `src/scenes/` and exports a factory function returning a `Scene` struct.

- **title_scene** — Title screen. ENTER to play, ESC to quit.
- **play_scene** — Main gameplay. Player moves with WASD/arrows, collects items, avoids enemies. Transitions to gameover when health <= 0.
- **gameover_scene** — Displays final score. ENTER to retry, ESC for title.

### Adding a new scene

1. Create `src/scenes/my_scene.h` and `src/scenes/my_scene.c`
2. Define static `init`/`update`/`draw`/`cleanup` callbacks, export a `Scene my_scene(void)` factory
3. Add the `.c` to `CMakeLists.txt`
4. Transition from any scene with `scene_set(my_scene())`

### Adding a new gameplay module

1. Create `src/module.h` and `src/module.c`
2. Add the `.c` to `CMakeLists.txt`
3. Use shared types from `platform.h` (`rectangle`, `color`, `vector2`) — don't introduce parallel aliases
4. Use `GameState *` from `gamestate.h` for cross-scene data rather than local statics

### Adding a new input action

Append to the `Action` enum in `input.h` (before `ACTION_COUNT`), then bind default keys in `input_init()` in `input.c`.

### Conventions

- Module state is encapsulated via file-scope `static` variables.
- Cross-scene data goes through `gamestate.h`, not scene-local statics.
- Draw functions follow the pattern: `clear_background()`, `begin_drawing()`, draw calls, `end_drawing()`.
- Text centering: `text_width_px = strlen(text) * 8 * scale`.
- Prefer the action-based input API (`action_down`/`action_pressed`) over raw key polling.

# AGENTS.md

Guidance for coding agents working in this repository.

## Project Overview

- This is a small C17 SDL3 game project built on a lightweight scene-based structure.
- SDL3 is vendored in `vendor/sdl/`.
- Game code lives in `src/`.
- The current game flow is `title_scene -> play_scene -> gameover_scene`.
- Rendering is still intentionally simple: SDL renderer primitives plus SDL's built-in debug text.
- There is no asset pipeline, sprite system, UI framework, or audio gameplay layer in the project code right now.

## Current Gameplay Snapshot

- `title_scene` shows the start screen. Confirm starts a run; cancel exits the app.
- `play_scene` creates the player, resets shared game state, spawns collectibles and enemies, and updates score/health during play.
- `gameover_scene` reads the shared `GameState`, displays the final score, and lets the player retry or return to title.
- Shared run data currently lives in `gamestate.c` as a single global `GameState` with `score` and `health`.

## Where To Work

- Primary code lives in `src/`.
- Scene implementations live in `src/scenes/`.
- Shared gameplay modules currently include `gamestate`, `enemy`, and `collectible`.
- Build configuration is in `CMakeLists.txt`.
- Window defaults live in `src/config.h`.
- Avoid editing `vendor/sdl/` unless the task explicitly requires changing the vendored dependency.

## Build And Run

Current local build directory is configured for Visual Studio 18 2026 on Windows.

```bash
cmake -S . -B build -G "Visual Studio 18 2026" -A x64 -Wno-dev
cmake --build build --config Release
build/Release/sdl_only.exe
```

Notes:

- If you add a new `.c` file, update `add_executable(...)` in `CMakeLists.txt`.
- Header-only changes do not require CMake edits.
- On Windows, CMake copies the SDL3 runtime next to the executable after build.
- There is no automated test suite yet, so validation is usually a successful build plus a quick manual run.

## Architecture

Module dependency flow:

```text
main.c -> config, platform, input, game, scene
game.c -> scene, scenes/title_scene
scenes/* -> platform, input, draw, scene
play_scene.c -> gamestate, collectible, enemy, other scenes
gameover_scene.c -> gamestate, other scenes
collectible.c -> draw, gamestate
enemy.c -> draw, gamestate, platform
draw.c -> platform
input.c -> platform
platform.c -> SDL3
scene.c -> no project-local dependencies
gamestate.c -> no project-local dependencies
```

Main modules:

- `platform.h/.c`: owns the SDL window and renderer, tracks delta time and FPS, and exposes shared typedefs like `rectangle`, `color`, and `vector2`.
- `input.h/.c`: action-based input layer with default bindings for movement, confirm, and cancel, plus optional raw keyboard and mouse queries.
- `draw.h/.c`: wrapper around SDL drawing calls for rectangles, lines, circles, and debug text.
- `scene.h/.c`: small scene manager. `scene_set(...)` cleans up the previous scene before initializing the next one.
- `game.h/.c`: chooses the starting scene and shuts the scene system down.
- `gamestate.h/.c`: stores shared run state such as score and health.
- `collectible.h/.c`: owns collectible storage, spawn/update/draw logic, and score/health rewards.
- `enemy.h/.c`: owns enemy storage, spawn/update/draw logic, simple movement behaviors, and contact damage.

## Scene Conventions

- Each scene exports a factory like `Scene play_scene(void)`.
- Scene callbacks are typically `static` functions for `init`, `update`, and `draw`.
- `cleanup` may be `NULL` when a scene does not own disposable state.
- Scene-local state is currently stored in file-level `static` variables.
- Scene transitions happen with `scene_set(other_scene())`.
- `draw()` functions usually call `clear_background(...)`, then `begin_drawing()`, perform draw calls, and finish with `end_drawing()`.
- Text centering is manual. The debug font is effectively `8x8` pixels per character before scaling.

## Gameplay Module Conventions

- `gamestate_reset()` is called at the start of a run from `play_scene`.
- `collectibles_init()` and `enemies_init()` clear their fixed-size arrays before new spawns.
- `collectible_spawn(...)` and `enemy_spawn(...)` append into static arrays up to `MAX_COLLECTIBLES` and `MAX_ENEMIES`.
- Collision checks are local to the gameplay modules right now; keep additions lightweight unless a broader collision system is explicitly needed.
- Prefer updating shared run data through `GameState *state = gamestate_get()` rather than introducing duplicate score/health state inside scenes.

## Input Conventions

- Default movement bindings are `WASD` and arrow keys.
- Confirm is bound to `Enter` and `Space`.
- Cancel is bound to `Escape`.
- Add new actions by extending the `Action` enum in `src/input.h` before `ACTION_COUNT`, then bind defaults in `input_init()` in `src/input.c`.
- Prefer `action_down(...)` and `action_pressed(...)` before reaching for raw key polling.

## Practical Guidelines

- Keep changes minimal and consistent with the current lightweight style.
- Prefer adding functionality through existing modules instead of bypassing them with raw SDL calls throughout scenes.
- Preserve the current scene lifecycle: cleanup old scene, init new scene, then update/draw each frame.
- Use the shared typedefs from `platform.h` (`rectangle`, `color`, `vector2`) instead of introducing parallel aliases.
- Reuse the existing fixed-array pattern for lightweight gameplay entities unless the task calls for a larger structural change.
- If you rename or add scenes/modules, update both `CMakeLists.txt` and this guide when the workflow or architecture meaningfully changes.

## Common Tasks

Adding a new scene:

1. Create `src/scenes/my_scene.h` and `src/scenes/my_scene.c`.
2. Implement scene callbacks and return a `Scene` from `my_scene()`.
3. Add the new `.c` file to `CMakeLists.txt`.
4. Transition to it with `scene_set(my_scene())`.

Adding a new gameplay module:

1. Add `module.h` and `module.c` under `src/`.
2. Include the header where needed.
3. Add the `.c` file to `CMakeLists.txt`.
4. Keep ownership boundaries clear: platform for SDL/window concerns, input for controls, draw for rendering helpers, gamestate for shared run data, and scenes for gameplay flow.

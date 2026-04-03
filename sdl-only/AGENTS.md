# AGENTS.md

Guidance for coding agents working in this repository.

## Project Overview

- This is a small C17 SDL3 game template.
- SDL3 is vendored in `vendor/sdl/`.
- Game code lives in `src/`.
- The current game flow is scene-based: the app starts in `title_scene`, then transitions to `play_scene`.
- Rendering is intentionally simple: SDL renderer primitives plus SDL's built-in debug text. There are currently no sprites, textures, audio systems, or asset pipelines in the game code.

## Where To Work

- Primary code lives in `src/`.
- Scene implementations live in `src/scenes/`.
- Build configuration is in `CMakeLists.txt`.
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
- On Windows, CMake already copies the SDL3 runtime next to the executable after build.
- There is no automated test suite yet, so validation is usually a successful build plus a quick manual run.

## Architecture

Module dependency flow:

```text
main.c -> platform, input, game, scene
game.c -> scene, scenes/*
scenes/* -> platform, input, draw, scene (+ other scenes for transitions)
draw.c -> platform
input.c -> platform (shared SDL types)
platform.c -> SDL3
scene.c -> no project-local dependencies
```

Main modules:

- `platform.h/.c`: owns the SDL window and renderer, tracks delta time and FPS, exposes shared engine-style types like `rectangle`, `color`, and `vector2`.
- `input.h/.c`: action-based input layer with default bindings for movement, confirm, and cancel. Use `action_down(...)` and `action_pressed(...)` before reaching for raw key polling.
- `draw.h/.c`: wrapper around SDL drawing calls for rectangles, lines, circles, and debug text.
- `scene.h/.c`: small scene manager. `scene_set(...)` cleans up the previous scene before initializing the next one.
- `game.h/.c`: chooses the starting scene and shuts the scene system down.
- `src/scenes/*.c`: scene-local behavior and transitions.

## Scene Conventions

- Each scene exports a factory like `Scene title_scene(void)`.
- Scene callbacks are typically `static` functions for `init`, `update`, `draw`, and `cleanup`.
- Scene-local state is currently stored in file-level `static` variables.
- Scene transitions happen with `scene_set(other_scene())`.
- `draw()` functions usually call `clear_background(...)`, then `begin_drawing()`, perform draw calls, and finish with `end_drawing()`.

## Input Conventions

- Default movement bindings are `WASD` and arrow keys.
- Confirm is bound to `Enter` and `Space`.
- Cancel is bound to `Escape`.
- Add new actions by extending the `Action` enum in `src/input.h` before `ACTION_COUNT`, then bind defaults in `input_init()` in `src/input.c`.

## Practical Guidelines

- Keep changes minimal and consistent with the current lightweight style.
- Prefer adding functionality through existing modules instead of bypassing them with raw SDL calls in many places.
- Preserve the current scene lifecycle: cleanup old scene, init new scene, then update/draw each frame.
- Use the shared typedefs from `platform.h` (`rectangle`, `color`, `vector2`) instead of introducing parallel aliases.
- When centering text, remember the debug font is effectively `8x8` pixels per character before scaling.

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
4. Keep ownership boundaries clear: platform for SDL/window concerns, input for controls, draw for rendering helpers, scenes for gameplay flow.

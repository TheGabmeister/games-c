# AGENTS.md

This file provides guidance to coding agents working in this repository.

## Build Commands

```powershell
# Configure (Visual Studio 18 2026 generator, x64)
cmake -S . -B build -G "Visual Studio 18 2026" -A x64 -Wno-dev

# Build
cmake --build build --config Debug

# Run
.\build\Debug\sdl_only.exe
```

Notes:

- The CMake project name is `sdl_only`, so the executable is `sdl_only.exe`.
- On Windows, `CMakeLists.txt` copies the SDL3 runtime DLL next to the built executable after build.

## Project Layout

SDL3 2D game template in C17 with modular architecture.

### `CMakeLists.txt`

- Sets `CMAKE_C_STANDARD` to `17`
- Builds SDL3 from `vendor/sdl` via `add_subdirectory()`
- Builds a single executable from all `.c` files in `src/`
- Links only `SDL3::SDL3`

When adding new `.c` files, add them to the `add_executable()` source list in `CMakeLists.txt`.

### Module Overview

| File | Purpose |
|---|---|
| `src/main.c` | Entry point. Calls `game_init()`, `game_run()`, `game_shutdown()`. |
| `src/config.h` | Game constants: window size, timestep, title. |
| `src/types.h` | Core types: `Vec2`, `Color`, `Rect` with inline helpers. |
| `src/game.h/c` | SDL lifecycle, main loop (fixed timestep), scene management. |
| `src/scene.h` | Scene interface (function pointer vtable: init/shutdown/update/render). |
| `src/input.h/c` | Keyboard and mouse state with down/pressed/released queries. |
| `src/draw.h/c` | Camera, world/screen-space primitives, text rendering. |
| `src/demo_scene.h/c` | Example scene: player movement, walls, coins, camera follow, HUD. |

### Architecture Patterns

**Game Loop** (`game.c`): Fixed timestep via accumulator. `input_update()` runs once per frame, `scene->update(FIXED_DT)` runs zero or more times per frame to catch up, `scene->render()` runs once per frame.

**Scene System** (`scene.h`): Each scene is a `const Scene` struct with function pointers. Scenes are switched via `game_set_scene()` which defers the transition to avoid mid-frame issues. The old scene's `shutdown` is called before the new scene's `init`.

**Input** (`input.h`): Snapshots SDL keyboard/mouse state each frame. Tracks previous frame state for edge detection (`input_key_pressed`, `input_key_released`).

**Drawing** (`draw.h`): All world-space drawing goes through a camera transform. Screen-space functions (`draw_*_screen`, `draw_text`) bypass the camera for HUD/UI. Text uses `SDL_RenderDebugText` (8x8 monospace) with optional scale.

### How to Add a New Scene

1. Create `src/my_scene.h` and `src/my_scene.c`
2. Implement the four callbacks (`init`, `shutdown`, `update`, `render`)
3. Export a `const Scene my_scene` struct
4. Add `src/my_scene.c` to `add_executable()` in `CMakeLists.txt`
5. Switch to it via `game_set_scene(&my_scene)` from any update callback

### `vendor/sdl`

- Vendored SDL3 source used by the build
- Treat this as third-party code unless the task explicitly requires patching SDL itself

## Agent Guidance

- Keep `src/main.c` minimal. It should only init, run, and shutdown.
- Each new feature or game screen should be its own scene in a separate file.
- Shared game state (player stats, inventory, etc.) can live in a dedicated module rather than in scene files.
- Prefer updating this documentation when architecture changes so `AGENTS.md` stays aligned with the real codebase.
- Avoid editing `vendor/sdl` unless the user explicitly asks for dependency-level changes.

# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Build Commands

```bash
cmake -S . -B build                       # Configure (first time)
cmake --build build --config Debug        # Debug build
cmake --build build --config Release      # Release build
```

Output lands in `build/Debug/pacman.exe` or `build/Release/pacman.exe`. Resources are copied automatically next to the executable.

## Architecture

C11 game project using **SDL3** for platform/rendering, **Flecs** for ECS, **Chipmunk2D** for physics, **Nuklear** for immediate-mode GUI.

### Core Loop

`main.c` calls three functions on the game manager: `game_manager_init()` → `game_manager_loop()` → `game_manager_fini()`.

The game loop polls SDL events, then calls `ecs_progress(world, delta)` each frame, which drives all Flecs systems in phase order.

### Managers vs Systems

**Managers** (`src/managers/`) own resources and initialization. Each manager registers its resources, components, or systems with the Flecs world during `game_manager_init()`. They provide getter functions (e.g., `texture_manager_get(TEXTURE_SHIP)`) and register cleanup via `ecs_atfini()`.

**Systems** (`src/systems/`) contain per-frame logic. They are Flecs systems declared in `system_manager_init()` with phase scheduling and query signatures. They iterate matching entities using `ecs_field()`.

### Frame Pipeline (defined in `managers/system.c`)

| Phase | Systems |
|-------|---------|
| EcsOnLoad | display refresh, time progression, input processing |
| EcsPostLoad | GUI input forwarding |
| EcsPreUpdate | GUI widget update, transitions, state machine |
| EcsOnUpdate | viewport, scene update |
| EcsOnValidate | physics step, collision detection |
| EcsPreStore | music/sound playback |
| EcsOnStore | animation, rendering (scene → viewports → physical → images → labels → GUI → composite), debug overlays |

### Scenes

Scenes (`src/scenes/`) follow a four-function pattern: `spawn_*`, `init_*`, `update_*`, `fini_*`. Each scene is registered in the `_targets` table in `systems/scene.c`. Scene flow: SPLASH → TITLE → LEVEL → TITLE.

Scenes create a root entity with `Scene`, `Stateful`, and `Transition` components. Child entities are spawned under it. The `Stateful` component drives a state machine (CREATED → STARTING → RUNNING → STOPPING → STOPPED) that gates scene lifecycle.

### Assets

All assets use enum-based fixed registries — add a new enum value and a `_load()` call in the corresponding manager. Asset types: `TextureName`, `SoundName`, `MusicName`, `FontName`, `DataName`. Assets live in `res/` and are accessed via relative paths (`./res/...`).

### Platform Layer

`src/platform.h/.c` wraps SDL3 for window management, input polling (keyboard/mouse), frame timing, and renderer access. It was previously a separate shared library (`vendor/engine`), now compiled directly into the executable.

### Physics

`physics_manager_init()` creates a `cpSpace` stored in the `Physics` singleton. Helper functions (`physics_ball`, `physics_line`, `physics_box`, `physics_wedge`) create Chipmunk bodies/shapes. The `Physical` component holds `cpBody*` and `cpShape*` per entity.

### GUI

Nuklear context is stored in the `Interface` singleton. Helper functions (`gui_window`, `gui_button`, `gui_slider`, `gui_label`) wrap Nuklear calls. UI is defined via `Widget` component entities with callbacks.

## Flecs Conventions

- `ecs_field()` uses **0-based** indexing over ALL query terms (tags + components). Tags count toward the index but cannot be fetched.
- Singletons: set with `ecs_singleton_set()`, read with `ecs_singleton_get()`.
- Components are registered centrally in `managers/component.c` using `ECS_COMPONENT_DEFINE()`.
- Key singletons: `Input`, `Settings`, `Debug`, `Display`, `Physics`, `Time`, `Interface`.

## Naming Conventions

- Types: `PascalCase`. Functions: `snake_case`. Enum values: `UPPER_CASE`.
- Internal/static helpers: `_underscore_prefix()`.
- File pairs: `name.c` + `name.h`.
- Manager functions: `*_manager_init()`, `*_manager_get()`, `*_manager_fini()`.

# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Build Commands

```bash
# Configure (from project root)
cmake -B build

# Build
cmake --build build

# Run (executable is in build/Debug/ on Windows)
./build/Debug/pong.exe
```

CMake auto-globs all `src/**/*.c` files — no need to update CMakeLists.txt when adding new source files.

## Architecture

Pong clone in C11 using **Flecs ECS** and **SDL3** (with SDL3_mixer for audio, SDL3_ttf for fonts). All dependencies are vendored in `vendor/`.

### Layers

- **platform.c/h** — SDL window/renderer lifecycle, delta time, global `vector2`/`vector3`/`vector4` typedefs. All rendering goes through `get_renderer()`.
- **event_bus.c/h** — Simple publish/subscribe system (not ECS). Used for cross-cutting events like `EVENT_GOAL_SCORED` and `EVENT_PLAY_SOUND` that trigger side effects (score tracking, audio).
- **managers/** — Bootstrap the ECS world at startup. Called in order: `component_manager_init` → `input_manager_init` → `entity_manager_init` → `system_manager_init`. Each registers Flecs components, tags, or systems.
- **components/** — Pure data structs with `ECS_COMPONENT_DECLARE` in headers and `ECS_COMPONENT_DEFINE` in `component.c`. Components are declared globally so any file can reference them after including the header.
- **systems/** — Flecs system callbacks (`void fn(ecs_iter_t *it)`). Registered in `system_manager_init` with explicit pipeline phases (EcsOnLoad → EcsPreUpdate → EcsOnUpdate → EcsPostUpdate → EcsOnValidate → EcsPreStore → EcsOnStore).

### Game Loop

`game_loop()` polls SDL events, calls `ecs_progress()` (which runs all registered systems in phase order), then calls `ui_render()` and `SDL_RenderPresent()`.

### ECS Conventions

- `ecs_field()` uses **0-based** indexing over all query terms (tags and components both count toward the index, but you can only call `ecs_field` on components).
- Input is stored as an **ECS singleton** (`Input`), not per-entity. Systems read it via `ecs_singleton_get(world, Input)`.
- Collision is a **component added/removed each frame** — `collision_clear` removes all `Collision` components in EcsPreUpdate, `collision_detect` re-adds them in EcsPostUpdate using layer/mask filtering.

### Scoring and Audio

Both use the event bus, not ECS. `score_init()` subscribes to `EVENT_GOAL_SCORED`; `audio_init()` subscribes to `EVENT_PLAY_SOUND`. Systems publish these events directly.

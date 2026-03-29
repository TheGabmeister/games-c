# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Build Commands

```bash
# Configure (uses Visual Studio generator via CMakePresets.json)
cmake --preset default

# Build
cmake --build build

# Run (executable is in build/Debug/ on Windows)
./build/Debug/adventure.exe
```

CMake auto-globs all `src/**/*.c` files — no need to update CMakeLists.txt when adding new source files.

## Architecture

Adventure (Atari 2600 reimagining) in C11 using **Flecs ECS** and **SDL3** (with SDL3_mixer for audio, SDL3_ttf for fonts). All dependencies are vendored in `vendor/`.

### Layers

- **platform.c/h** — SDL window/renderer lifecycle, delta time, global `vector2`/`vector3`/`vector4` typedefs. All rendering goes through `get_renderer()`.
- **event_bus.c/h** — Simple publish/subscribe system (not ECS). Used for cross-cutting events like `EVENT_PLAY_SOUND`, `EVENT_PLAYER_DIED`, `EVENT_DRAGON_KILLED`, `EVENT_GATE_STATE_CHANGED`, `EVENT_GAME_WON`.
- **managers/** — Bootstrap the ECS world at startup. Called in order: `component_manager_init` → `input_manager_init` → `entity_manager_init` → `system_manager_init`. Each registers Flecs components, tags, or systems.
- **components/** — Pure data structs with `ECS_COMPONENT_DECLARE` in headers and `ECS_COMPONENT_DEFINE` in `component.c`. Components are declared globally so any file can reference them after including the header.
- **systems/** — Flecs system callbacks (`void fn(ecs_iter_t *it)`). Registered in `system_manager_init` with explicit pipeline phases (EcsOnLoad → EcsPreUpdate → EcsOnUpdate → EcsPostUpdate → EcsOnValidate → EcsPreStore → EcsOnStore).
- **data/** — Static game data. `rooms.c/h` defines 30 rooms with exits, walls, and bridge sockets. `game_modes.c/h` defines spawn tables for 3 game modes.
- **menu.c/h** — Title screen rendering and input handling.

### Game Loop

`game_loop()` polls SDL events, handles scene input, calls `_scene_update()` for timers, calls `ecs_progress()` (which runs all registered systems in phase order), then calls `ui_render()` and `SDL_RenderPresent()`.

### Scenes

Game state is managed via `GameState` struct (non-ECS): `SCENE_MENU`, `SCENE_PLAYING`, `SCENE_PAUSED`, `SCENE_DEATH_FREEZE`, `SCENE_VICTORY`. Systems early-out when scene != SCENE_PLAYING. Render systems continue during all scenes.

### ECS Conventions

- `ecs_field()` uses **0-based** indexing over all query terms (tags and components both count toward the index, but you can only call `ecs_field` on components).
- Input is stored as an **ECS singleton** (`Input`), not per-entity. Systems read it via `ecs_singleton_get(world, Input)`.
- Collision is a **component added/removed each frame** — `collision_clear` removes all `Collision` components in EcsPreUpdate, `collision_detect` re-adds them in EcsPostUpdate using layer/mask filtering. Only entities in the same room collide.

### System Pipeline

| Phase | System | Purpose |
|-------|--------|---------|
| EcsOnLoad | `process_input` | Read keyboard into Input singleton |
| EcsPreUpdate | `collision_clear` | Remove transient collision data |
| EcsOnUpdate | `player_intent` | Convert input to player velocity |
| EcsOnUpdate | `dragon_ai` | Dragon behavior state machine |
| EcsOnUpdate | `bat_ai` | Bat wandering and item swaps |
| EcsOnUpdate | `magnet_pull` | Pull 2 nearest movable entities toward magnet |
| EcsOnUpdate | `carry_sync` | Attach carried entities to carriers |
| EcsOnUpdate | `move_entities` | Apply velocity with wall collision |
| EcsPostUpdate | `collision_detect` | Populate Collision components |
| EcsOnValidate | `resolve_player_interactions` | Sword kills, bites, pickup/drop |
| EcsOnValidate | `resolve_gate_state` | Open/close gates based on key presence |
| EcsOnValidate | `room_transition` | Move entities between rooms via exits |
| EcsOnValidate | `check_victory` | Detect chalice in throne room |
| EcsPreStore | `render_clear` | Clear window |
| EcsOnStore | `render_room` | Draw room walls, gates |
| EcsOnStore | `render_entities` | Draw player, dragons, bat, items |
| EcsOnStore | `render_fog` | Dark room fog overlay |
| EcsOnStore | `render_particles` | Draw particle effects |
| EcsOnStore | `render_colliders` | Debug collider overlay (F1) |

### Audio

Audio is event-driven via the event bus. `audio_init()` subscribes to `EVENT_PLAY_SOUND`. Sound files expected in `assets/sfx/`.

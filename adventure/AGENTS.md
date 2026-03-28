# AGENTS.md

Guidance for coding agents working in this repository.

## Project Summary

This is a Pong clone written in C11 using Flecs ECS and SDL3. Audio uses SDL3_mixer and UI text uses SDL3_ttf. Third-party dependencies are vendored under `vendor/`.

Primary game code lives in `src/`. Generated artifacts live in `build/`.

## Build And Run

From the project root:

```bash
cmake --preset default
cmake --build build
```

On Windows, the executable is typically:

```bash
build/Debug/pong.exe
```

The default preset is defined in `CMakePresets.json` and currently uses the Visual Studio generator on Windows.

CMake auto-globs `src/**/*.c`, so adding a new source file under `src/` usually does not require editing `CMakeLists.txt`.

## Working Areas

- `src/main.c`: minimal entry point that calls `game_init()`, `game_loop()`, and `game_fini()`.
- `src/game.c`: bootstraps SDL, the event bus, score and audio managers, the Flecs world, and the initial level.
- `src/platform.c/h`: SDL window and renderer lifecycle, frame timing, and shared vector types.
- `src/event_bus.c/h`: lightweight non-ECS publish/subscribe layer for cross-cutting events.
- `src/managers/`: ECS bootstrap and registration for components, entities, systems, input, and audio.
- `src/components/`: pure ECS data definitions.
- `src/systems/`: Flecs systems scheduled into explicit pipeline phases.
- `src/ui.c/h`: UI rendering layered on top of the ECS-driven frame.
- `assets/`: runtime assets copied beside the executable during the build.

## Architecture Notes

- The main loop polls SDL events, advances the ECS world with `ecs_progress()`, renders the UI, and presents the SDL renderer.
- Startup is defensive: `game_init()` returns `bool`, fails fast if SDL window creation or ECS initialization fails, and cleans up partial initialization before returning.
- Manager initialization order in `game_init()` matters:
  `component_manager_init()`, `input_manager_init()`, `entity_manager_init()`, then `system_manager_init()`.
- Input is modeled as an ECS singleton, not as per-entity input components.
- Collision is rebuilt each frame: one system clears collision markers early, another detects and re-adds them later in the pipeline.
- Scoring and audio are event-bus driven rather than ECS-driven. Systems publish events such as goal and sound events, and subscribers handle the side effects.
- The event bus is intentionally small and synchronous. It validates event types and callbacks and logs when the fixed subscriber capacity is exceeded.

## Agent Guardrails

- Prefer changing files under `src/` and `assets/`.
- Do not edit `vendor/` unless the task is specifically about a vendored dependency.
- Do not rely on `build/` contents being stable source of truth; regenerate with CMake when needed.
- If you add gameplay code, keep data in components and behavior in systems or managers rather than mixing both into one place.
- If you add rendering code, route it through the SDL renderer provided by `get_renderer()`.
- Keep failure paths explicit and safe. If you add startup or shutdown logic, preserve the current fail-fast and partial-cleanup behavior.

## Verification

There is no dedicated automated test suite in the repo at the moment. The default verification path is:

```bash
cmake --preset default
cmake --build build
```

`CMakeLists.txt` enables a stricter warning baseline for the game target. There is also an optional `PONG_ENABLE_ASAN` CMake option for AddressSanitizer builds when supported by the active toolchain.

If the task changes runtime behavior, also run the built executable and sanity-check the affected gameplay or UI path.

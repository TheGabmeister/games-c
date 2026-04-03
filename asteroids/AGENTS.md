# AGENTS.md

## Scope
These instructions apply to the entire repository.

## Project Overview
- This is a small C game project built with CMake.
- The top-level build pulls in `raylib` 5.5 with `FetchContent`.
- The game now uses a small multi-file architecture under `src/` with a shared `GameContext`.
- `CMAKE_EXPORT_COMPILE_COMMANDS` is enabled at the top level.

## Repository Layout
- `CMakeLists.txt`: the only CMake file in the repo; it handles dependency setup, source discovery under `src/`, output directories, `compile_commands.json`, and resource copy steps.
- `src/game.c`: application entrypoint and main loop orchestration.
- `src/game_types.h`: shared runtime state and gameplay constants, including `GameContext`.
- `src/game_flow.c`: high-level title, playing, and game-over flow.
- `src/input.c`, `src/ship.c`, `src/bullets.c`, `src/asteroids.c`, `src/collisions.c`, `src/particles.c`, `src/background.c`, `src/ui.c`, `src/world.c`, `src/render_fx.c`: focused gameplay and rendering systems.
- `src/resources/`: runtime assets copied next to the built executable, currently including `coin.wav` and `mecha.png`.
- `build/`: local build output; treat as generated content.

## Build And Run
- Configure: `cmake -S . -B build`
- Build: `cmake --build build`
- Run the desktop build from `build/asteroids` or `build/asteroids/<Config>` on multi-config generators.
- The top-level CMake also contains an optional `PLATFORM=Web` path with a different resource-copy destination and `.html` output suffix.

## Implementation Notes
- Keep gameplay code in `src/` and assets in `src/resources/`.
- When adding new source or header files under `src/`, the root `CMakeLists.txt` will pick them up automatically through its `GLOB_RECURSE` source list.
- If you add files that must be present at runtime, place them under `src/resources/` so the existing copy step keeps working.
- If you change where assets live, update both resource copy branches in the root `CMakeLists.txt` for desktop and Web builds.
- Prefer extending the existing module split and passing `GameContext *` explicitly rather than reintroducing file-scope shared globals.

## Code Style
- Match the existing C style across `src/`: simple functions, `static` helpers for internal logic, and straightforward naming.
- Prefer small focused helpers over deeply nested logic.
- Keep comments brief and only where intent is not obvious from the code.
- Avoid introducing new dependencies unless the user asks for them.

## Validation
- There are no automated tests in the repo right now.
- After code changes, validate by rebuilding with CMake.
- For gameplay, rendering, input, or asset-loading changes, launch the game and verify the behavior manually.

## Agent Guidelines
- Do not edit generated files under `build/` unless the user explicitly asks for it.
- Do not treat `CLAUDE.md` as authoritative when it conflicts with the current code; prefer the checked-in source and CMake files.
- Prefer minimal, targeted changes over broad refactors.
- Preserve the current CMake structure unless the task requires a build-system change.

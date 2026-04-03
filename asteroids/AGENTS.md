# AGENTS.md

## Scope
These instructions apply to the entire repository.

## Project Overview
- This is a small C game project built with CMake.
- The top-level build pulls in `raylib` 5.5 with `FetchContent`.
- The game is currently implemented as a single-source desktop build centered in `src/game.c`.
- `CMAKE_EXPORT_COMPILE_COMMANDS` is enabled at the top level.

## Repository Layout
- `CMakeLists.txt`: top-level build, dependency setup, output directories, `compile_commands.json`, and resource copy steps.
- `src/CMakeLists.txt`: automatically includes all `*.c` and `*.h` files under `src/`.
- `src/game.c`: current game loop, state management, input mapping, rendering, collisions, and wave progression.
- `src/resources/`: runtime assets copied next to the built executable, currently including `coin.wav` and `mecha.png`.
- `build/`: local build output; treat as generated content.

## Build And Run
- Configure: `cmake -S . -B build`
- Build: `cmake --build build`
- Run the desktop build from `build/asteroids` or `build/asteroids/<Config>` on multi-config generators.
- The top-level CMake also contains an optional `PLATFORM=Web` path with a different resource-copy destination and `.html` output suffix.

## Implementation Notes
- Keep gameplay code in `src/` and assets in `src/resources/`.
- When adding new source or header files under `src/`, `src/CMakeLists.txt` will pick them up automatically.
- If you add files that must be present at runtime, place them under `src/resources/` so the existing copy step keeps working.
- If you change where assets live, update both resource copy branches in the root `CMakeLists.txt` for desktop and Web builds.
- Prefer extending the existing small `static` helper pattern in `src/game.c` rather than introducing large subsystems unless the task calls for it.

## Code Style
- Match the existing C style in `src/game.c`: simple functions, `static` helpers for internal logic, and straightforward naming.
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

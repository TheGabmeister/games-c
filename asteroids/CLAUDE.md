# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Build and Run

```bash
cmake -S . -B build        # Configure (downloads raylib 5.5 via FetchContent)
cmake --build build         # Build
build/asteroids/asteroids.exe  # Run (Windows)
```

On multi-config generators (e.g. Visual Studio), the executable may be under `build/asteroids/<Config>/`.

There are no automated tests. Validate changes by rebuilding and running the game manually.

## Architecture

Single-file C game using **raylib 5.5**. All gameplay logic is in `src/game.c`, which currently implements an action-based input system (keyboard + gamepad) with a basic game loop. The actual asteroids gameplay has not yet been built on top of this foundation.

**Input abstraction:** `ActionType` enum maps logical actions (UP, DOWN, LEFT, RIGHT, FIRE) to physical keys/buttons via `ActionInput` structs. `IsActionPressed`/`IsActionReleased`/`IsActionDown` wrap raylib's input functions to support both keyboard and gamepad transparently.

## Adding Code and Assets

- New `.c` and `.h` files in `src/` are picked up automatically by CMake (`GLOB_RECURSE`).
- Runtime assets go in `src/resources/` — a post-build step copies them next to the executable.
- If you change where assets live, update the copy commands in the root `CMakeLists.txt`.

## Code Style

- Match the existing C style: simple functions, `static` helpers for internal logic, straightforward naming.
- Prefer small focused helpers over deeply nested logic.
- Do not introduce new dependencies unless asked.
- Do not edit generated files under `build/`.

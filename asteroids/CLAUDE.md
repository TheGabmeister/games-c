# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Build and Run

```bash
cmake -S . -B build        # Configure (downloads raylib 5.5 via FetchContent)
cmake --build build         # Build
build/asteroids/asteroids.exe  # Run (Windows)
```

On multi-config generators (e.g. Visual Studio), the executable may be under `build/asteroids/<Config>/`.

**Note:** Building from a terminal requires the MSVC environment to be set up (run `vcvarsall.bat x64` first, or use a Developer Command Prompt). The VS Code CMake Tools extension handles this automatically.

There are no automated tests. Validate changes by rebuilding and running the game manually.

## Architecture

Single-file C game using **raylib 5.5** (with `raymath.h` for vector math). All code lives in `src/game.c`.

**Game state machine:** `GameState` enum drives three modes — `STATE_TITLE`, `STATE_PLAYING`, `STATE_GAMEOVER`. The main loop dispatches update/draw per state.

**Entity model:** Fixed-size static arrays with `active`/`alive` flags (no dynamic allocation). Pool sizes: 30 bullets, 64 asteroids, 256 particles, 100 stars.

**Key subsystems in `src/game.c`:**
- **Input abstraction** — `ActionType` enum maps logical actions (UP, DOWN, LEFT, RIGHT, FIRE) to keyboard keys and gamepad buttons via `ActionInput` structs. `IsActionPressed`/`IsActionDown` wrap raylib input to support both transparently. TAB toggles between WASD and arrow key layouts.
- **Ship** — Rotation + thrust physics with drag (velocity *= 0.98/frame). Screen wrapping. Invulnerability with blink effect after respawn.
- **Asteroids** — Irregular polygons (8-12 vertices with random radius perturbation). Three sizes that split on destruction (large → 2 medium → 2 small). Wave progression spawns from screen edges.
- **Collisions** — Circle-based via `CheckCollisionCircles` for bullet-asteroid and ship-asteroid interactions.
- **Particle system** — Explosions, thrust exhaust. Particles fade via alpha over lifetime.
- **Visual effects** — Neon glow (double-draw: thick/dim + thin/bright), parallax starfield, screen shake via `Camera2D`, thrust flame flicker.

## Adding Code and Assets

- New `.c` and `.h` files in `src/` are picked up automatically by CMake (`GLOB_RECURSE`).
- Runtime assets go in `src/resources/` — a post-build step copies them next to the executable.
- If you change where assets live, update the copy commands in the root `CMakeLists.txt`.

## Code Style

- Match the existing C style: simple functions, `static` helpers for internal logic, straightforward naming.
- Prefer small focused helpers over deeply nested logic.
- Do not introduce new dependencies unless asked.
- Do not edit generated files under `build/`.
- No textures or sprites — all rendering uses primitive shapes (lines, circles, triangles) via raylib draw functions.
- No sound or music.

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

C game using **raylib 5.5** (with `raymath.h` for vector math), organized into modules under `src/`.

**Central state:** All game state lives in a single `GameContext` struct (defined in `game_types.h`) that is passed by pointer to every subsystem. This includes the ship, entity arrays, input bindings, and game-flow state. `game_types.h` also defines all constants, enums, and entity structs — it is the shared dependency for all modules.

**Initialization:** `game.c` creates a zero-initialized `GameContext`, then calls `InputSetDefault` and `BackgroundInitStars` before entering the main loop. There is no single boot function — init is done inline in `main`.

**Game state machine:** `GameState` enum drives three modes — `STATE_TITLE`, `STATE_PLAYING`, `STATE_GAMEOVER`. The main loop in `game.c` dispatches update/draw per state. Session resets (new game) go through `InitGameSession` in `game_flow.c`, which preserves `highScore` and the starfield across sessions.

**Entity model:** Fixed-size static arrays with `active`/`alive` flags (no dynamic allocation). Pool sizes: 30 bullets, 64 asteroids, 256 particles, 100 stars.

**Input system:** `ActionInput` maps each logical action to multiple keyboard keys (WASD + arrow keys simultaneously) and a gamepad button. The `input` module loops over a `keys[]` array per action, so both layouts always work without toggling. Bindings are configured in `InputSetDefault`.

**Module layout** — each subsystem is a `.c/.h` pair under `src/`:

| Module | Responsibility |
|---|---|
| `game.c` | Entry point, main loop, state dispatch |
| `game_types.h` | All shared types, constants, `GameContext` |
| `game_flow` | State transitions, session init/reset, wave progression |
| `input` | Action abstraction over keyboard/gamepad; multiple keys per action |
| `ship` | Ship physics, drawing, thrust flame |
| `asteroids` | Asteroid spawning, movement, wave management, drawing |
| `bullets` | Bullet spawning, lifetime, drawing |
| `collisions` | Bullet-asteroid and ship-asteroid collision detection + response |
| `particles` | Particle spawning, update, drawing (explosions, exhaust) |
| `background` | Parallax starfield |
| `world` | Screen wrapping, screen shake, camera |
| `render_fx` | Neon glow draw helpers (lines, circles, triangles) |
| `ui` | HUD, title screen, game-over screen |

## Adding Code and Assets

- New `.c` and `.h` files in `src/` are picked up automatically by CMake (`GLOB_RECURSE`).
- Runtime assets go in `src/resources/` — a post-build step copies them next to the executable.
- If you change where assets live, update the copy commands in the root `CMakeLists.txt`.

## Code Style

- Match the existing C style: simple functions, `static` helpers for internal logic, straightforward naming (`ModuleVerb` convention, e.g. `ShipUpdate`, `BulletsDraw`).
- Every module function takes `GameContext *ctx` (or `const GameContext *ctx` for read-only) as its first parameter.
- Prefer small focused helpers over deeply nested logic.
- Do not introduce new dependencies unless asked.
- Do not edit generated files under `build/`.
- No textures or sprites — all rendering uses primitive shapes (lines, circles, triangles) via raylib draw functions and the `render_fx` neon helpers.
- No sound or music.

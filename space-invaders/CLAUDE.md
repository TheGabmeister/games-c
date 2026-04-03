# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Build

The project uses CMake with the Visual Studio 18 2026 generator:

```bash
# Configure (only needed once or after CMakeLists.txt changes)
cd build && cmake ..

# Build
cd build && cmake --build .
```

The executable is output to `build/space-invaders/space-invaders.exe`. There are no runtime resources — all rendering uses raylib primitives.

## Architecture

**Screen state machine** (`game.c`): Drives the flow `LOGO → TITLE → GAMEPLAY → ENDING`. Each screen implements 5 functions (`Init`, `Update`, `Draw`, `Unload`, `Finish`). Transitions use fade-in/fade-out alpha blending. `Finish` returns non-zero to trigger the next screen.

**Module split**:
- `game_types.h` — Central header defining all constants, enums, and structs. The `GameState` mega-struct holds all gameplay entities (player, aliens, bullets, shields, particles, etc.).
- `gameplay.h/c` — Core game logic: player input, alien marching/acceleration, enemy fire (time-based), collision resolution, wave progression, game-over conditions. Exposes 4 functions: `GameInit`, `GameUpdate`, `GameDraw`, `GameShouldEnd`.
- `drawing.h/c` — Primitive rendering helpers for all visual elements (ships, aliens, shields, bullets, HUD, starfield, floating text).
- `particles.h/c` — Fixed-pool particle system (512 particles, no dynamic allocation).
- `screen_gameplay.c` — Thin wrapper that delegates to `gameplay.*` and bridges `GameState` with `AppState`.

**Shared state** (`screens.h`): `AppState` struct (highScore, lastScore, lastWave) persists across screens within a session. Defined in `game.c`, accessed by title and ending screens.

## Design Constraints

- **No textures, custom fonts, or audio** — all visuals use raylib draw primitives and the default font.
- **Fixed-size pools only** — no heap allocation for gameplay entities. Excess spawns are skipped.
- **SPEC.md is the source of truth** for gameplay rules, layout coordinates, timing formulas, and acceptance criteria.

## Key Conventions

- Collision uses `CheckCollisionRecs()` with priority order: UFO → alien → shield for player bullets.
- Alien step timing: `stepInterval = 0.02 + (aliveCount/55.0) * 0.48`, scaled by wave multiplier.
- Screen shake is applied as an offset to gameplay rendering only (HUD stays stable).
- All movement and timers use `GetFrameTime()` (delta time), never frame counting.

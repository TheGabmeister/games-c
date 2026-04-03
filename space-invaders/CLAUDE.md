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

The executable is output to `build/space-invaders/space-invaders.exe`. There are no runtime resources; all rendering uses raylib primitives.

## Architecture

**Screen state machine** (`game.c`): Drives the flow `TITLE -> GAMEPLAY -> ENDING`. Each screen implements 5 functions (`Init`, `Update`, `Draw`, `Unload`, `Finish`). `game.c` owns the current screen and shared `AppState`, passes `dt` into screens, and handles time-based fade transitions.

**Module split**:
- `game_types.h` - Central header defining constants, enums, and fixed-size gameplay/presentation structs.
- `gameplay.h/c` - Core game rules and state updates. Input is passed in as a small `GameInput` struct so gameplay code does not poll raylib directly.
- `game_renderer.h/c` - Gameplay rendering and screen-shake application for the active `GameState`.
- `drawing.h/c` - Primitive-only shape helpers for ships, aliens, shields, bullets, and HUD elements.
- `effects.h/c` - Visual-only starfield and floating-score helpers shared by gameplay and menu screens.
- `particles.h/c` - Fixed-pool particle system (512 particles, no dynamic allocation).
- `screen_gameplay.c` - Thin wrapper that reads raylib input, updates `GameState`, and copies score data into `AppState`.

**Shared state** (`screens.h`): `AppState` (highScore, lastScore, lastWave) persists across screens within a session. It is owned by `game.c` and passed explicitly into screen functions.

## Design Constraints

- **No textures, custom fonts, or audio** - all visuals use raylib draw primitives and the default font.
- **Fixed-size pools only** - no heap allocation for gameplay entities. Excess spawns are skipped.

## Key Conventions

- Collision uses `CheckCollisionRecs()` with priority order: UFO -> alien -> shield for player bullets.
- Alien step timing: `stepInterval = 0.02 + (aliveCount/55.0) * 0.48`, scaled by wave multiplier.
- Screen shake is applied as an offset to gameplay rendering only (HUD stays stable).
- All movement and timers are driven by delta time, never frame counting.

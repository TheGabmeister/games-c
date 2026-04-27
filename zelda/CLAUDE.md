# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project

A 2D top-down action-adventure game inspired by The Legend of Zelda (1986), built in C with raylib. The project is in early development — most game systems are stubs.

Key documents:
- **SPEC.md** — complete game design specification (mechanics, items, enemies, dungeons, tuning values). This is the source of truth for game behavior.
- **IMPLEMENTATION.md** — code architecture, data structures, module design, file formats. Read this before implementing any new system.
- **AGENTS.md** — additional guidance for coding agents (asset pipeline details).

## Build

```bash
# Build (uses existing build directory)
cmake --build build

# Full reconfigure + build
cmake -S . -B build
cmake --build build

# Release build
cmake --build build --config Release
```

The executable lands in `build/zelda/Debug/zelda.exe` (or `Release/`). Assets from `src/assets/` are copied to the executable's directory at build time, so runtime paths are relative: `assets/coin.wav`.

Generator: Visual Studio 18 2026, x64.

## Architecture

Window: 1024x960. Logical resolution: 1024x960 (1:1, no scaling). Tile size: 64x64 pixels.

Single `Game` struct holds all state (defined in `game.h`). The main loop in `main.c` calls `game_init` → `game_update`/`game_draw` per frame → cleanup on exit.

Current modules (header/source pairs):
- **game_config.h** — canonical source for all compile-time constants (window size, logical resolution, tile size, FPS, lives, deadzone). Other headers include this instead of defining their own constants.
- **game.c/.h** — Game struct definition, init/update/draw. Includes `game_config.h`.
- **input.c/.h** — keyboard + gamepad abstraction (WASD, arrows, gamepad with deadzone)
- **sounds.c/.h** — sound loading/playback (stub)
- **textures.c/.h** — texture management (stub)

See IMPLEMENTATION.md for the full planned module structure (enemy/, tilemap, collision, event system, etc.), data-driven enemy design with function pointers, screen file format, and other systems not yet implemented.

## Coding principles

- **C game programming best practices** — prefer stack/static allocation for fixed-size data, use heap when the size varies at runtime. Keep hot data contiguous, avoid unnecessary indirection.
- **KISS** — simplest thing that works. No clever patterns where a plain `if` does the job. But a plain `if` that must be copy-pasted into every new feature is not simple — it's a maintenance trap.
- **YAGNI** — don't build for hypothetical needs. No abstraction layers "for later."
- **DRY** — remove real duplication, not shape-similar code. Wrong abstraction costs more than repetition.
- **Locality of change** — adding a new entity, tile, or feature should require changes in as few files as possible. Prefer data-driven dispatch (flags, vtables) over centralized type switches when the set of types is expected to grow. If a new enemy requires editing the orchestrator, the abstraction is missing.
- **Dispatch strategy** — function pointers (vtable-style def tables) for large/growing type sets (enemies, projectiles). Switch statements for small/stable state machines (game state, player state). See IMPLEMENTATION.md for details.

When in doubt: for code one person owns and rarely changes, lean KISS. For interfaces many contributors touch, lean locality of change.

## Asset Pipeline

- **Sprites**: spritesheets drawn as single SVGs with all frames on a 64px grid, exported as one PNG via Inkscape (`inkscape player.svg -o player.png -w 256 -h 256` for a 4x4 sheet). One sheet per category (player, enemy type, tiles, items, etc.). Code indexes frames by row/column source rectangle. Store SVG and PNG in `src/assets/sprites/`.
- **Sounds**: generate with rfxgen (`"D:/rfxgen_v5.0_win_x64/rfxgen.exe" -g coin -o sound.wav`). Presets: coin, laser, explosion, powerup, hit, jump, blip. Store WAV in `src/assets/`.
- **Screen data**: plain text files, one per screen/room. See IMPLEMENTATION.md for format.
- Sound loading is resilient — missing files are skipped.

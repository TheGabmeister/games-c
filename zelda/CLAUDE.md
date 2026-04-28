# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project

A 2D top-down action-adventure game inspired by The Legend of Zelda (1986), built in C with raylib. Phases 1 (Player and Tiles) and 2 (World Navigation) are complete. Phase 3 (Combat) is next.

Key documents:
- **SPEC.md** — complete game design specification (mechanics, items, enemies, dungeons, tuning values). Source of truth for game behavior.
- **IMPLEMENTATION.md** — code architecture, data structures, module design, file formats, phased implementation plan with test checklists. Read this before implementing any new system.
- **AGENTS.md** — additional guidance for coding agents (asset pipeline details, rfxgen commands).

## Build

```bash
# Build (uses existing build directory)
cmake --build build

# Full reconfigure + build (needed when adding new .c/.h files)
cmake -S . -B build
cmake --build build

# Release build
cmake --build build --config Release
```

Generator: Visual Studio 18 2026, x64. Executable: `build/zelda/Debug/zelda.exe`. Assets from `src/assets/` are copied beside the executable at build time — runtime paths are relative (e.g., `assets/sprites/player.png`). CMake uses `GLOB_RECURSE` with `CONFIGURE_DEPENDS` to discover source files, but adding new files may require a reconfigure.

## Architecture

Window: 1024x960. Logical resolution: 1024x960 (1:1, no scaling). Tile size: 64x64 pixels.

Screen layout (top to bottom): HUD 1024x224 (3.5 tiles), 32px divider, play area 1024x704 (16x11 tiles). Play area starts at y=256 (`PLAY_AREA_Y`).

Single `Game` struct holds all state (defined in `game.h`). The main loop in `main.c` calls `game_init` → `game_update`/`game_draw` per frame → cleanup on exit. Game state machine: `STATE_PLAY` (normal gameplay), `STATE_TRANSITION` (scroll/fade between screens).

Implemented modules:
- **game_config.h** — canonical source for all compile-time constants. Other headers include this instead of defining their own.
- **game.c/.h** — Game struct, state machine, screen loading by grid coordinates, edge detection, scroll/fade transition orchestration, music update.
- **player.c/.h** — Player struct (pos, facing, state, health, inventory, animation), 4-directional movement with axis-aligned grid assist (perpendicular axis snaps to tile grid), separate-axis tile collision. Direction/PlayerState/Inventory types defined here.
- **tilemap.c/.h** — TileDef table, Screen struct (11x16 tile grid + warps), screen file parser (plain text with metadata), tile drawing with offset support, tile collision, warp lookup. TileType/ItemID/Warp types defined here.
- **anim.c/.h** — AnimDef (static definition: first_frame, count, duration, loops) and Anim (runtime: timer, current_frame, finished). Used by player and future enemies/tiles.
- **camera.c/.h** — TransitionCamera struct, scroll transitions (~30 frames) and fade-to-black transitions (~15+15 frames). Computes screen offsets and fade alpha.
- **input.c/.h** — keyboard + gamepad abstraction (WASD, arrows, d-pad, analog stick with deadzone).
- **textures.c/.h** — TextureID enum, global textures[] array, load/unload, frame rect helper.
- **hud.c/.h** — HUD rendering: hearts (half-heart granularity), rupees, keys, bombs, minimap grid showing current overworld position.
- **sounds.c/.h** — sound loading/playback + music streaming. Resilient to missing files.

Overworld grid: 16x8 screens, files named `assets/screens/XX_YY.txt`. Currently 5 test screens exist around position (7,4).

See IMPLEMENTATION.md for the full planned module structure and phased implementation plan. Follow the phases in order — each produces a testable build.

## Coding Principles

- **C game programming best practices** — prefer stack/static allocation for fixed-size data, use heap when the size varies at runtime. Keep hot data contiguous, avoid unnecessary indirection.
- **KISS** — simplest thing that works. No clever patterns where a plain `if` does the job. But a plain `if` that must be copy-pasted into every new feature is not simple — it's a maintenance trap.
- **YAGNI** — don't build for hypothetical needs. No abstraction layers "for later."
- **DRY** — remove real duplication, not shape-similar code. Wrong abstraction costs more than repetition.
- **Locality of change** — adding a new entity, tile, or feature should require changes in as few files as possible. Prefer data-driven dispatch (flags, vtables) over centralized type switches when the set of types is expected to grow. If a new enemy requires editing the orchestrator, the abstraction is missing.
- **Dispatch strategy** — function pointers (vtable-style def tables) for large/growing type sets (enemies, projectiles). Switch statements for small/stable state machines (game state, player state). See IMPLEMENTATION.md for details.

When in doubt: for code one person owns and rarely changes, lean KISS. For interfaces many contributors touch, lean locality of change.

## Asset Pipeline

- **Sprites**: spritesheets drawn as single SVGs with all frames on a 64px grid, exported as one PNG via Inkscape (`"C:/Program Files/Inkscape/bin/inkscape.exe" player.svg -o player.png -w 256 -h 256` for a 4x4 sheet). One sheet per category (player, enemy type, tiles, items, etc.). Code indexes frames by row/column source rectangle. Store SVG and PNG in `src/assets/sprites/`.
- **Sounds**: generate with rfxgen (`"D:/rfxgen_v5.0_win_x64/rfxgen.exe" -g coin -o sound.wav`). Presets: coin, laser, explosion, powerup, hit, jump, blip. Store WAV in `src/assets/`.
- **Music**: OGG files in `src/assets/music/`. Loaded via `LoadMusicStream`, updated every frame.
- **Screen data**: plain text files in `src/assets/screens/` (overworld), `src/assets/dungeons/<n>/` (dungeons), `src/assets/caves/` (caves). Format: optional metadata lines (`warp:`, `enemy:`, `item:`, `#` comments), then 11 rows of 16 tile characters (W=wall, .=floor, ~=water, D=door, P=pushblock, S=stairs). See IMPLEMENTATION.md for full format.
- Sound and music loading is resilient — missing files are skipped.

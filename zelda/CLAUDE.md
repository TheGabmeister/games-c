# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project

A 2D top-down action-adventure game inspired by The Legend of Zelda (1986), built in C with raylib. Phases 1 (Player and Tiles), 2 (World Navigation), and 3 (Combat) are complete. Phase 4 (Inventory/Progression) is next.

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

Strict compiler warnings are enabled (`/W4` on MSVC, `-Wall -Wextra` on GCC/Clang). Implicit function declarations, incompatible pointer types, and missing return values are promoted to errors. This is critical in C — a missing `#include` for a function like `Clamp` silently breaks float argument passing on x64 rather than failing to compile.

## Architecture

Window: 1024x960. Logical resolution: 1024x960 (1:1, no scaling). Tile size: 64x64 pixels.

Screen layout (top to bottom): HUD 1024x224 (3.5 tiles), 32px divider, play area 1024x704 (16x11 tiles). Play area starts at y=256 (`PLAY_AREA_Y`).

Single `Game` struct holds all state (defined in `game.h`). The main loop in `main.c` calls `game_init` -> `game_update`/`game_draw` per frame -> cleanup on exit. Game state machine: `STATE_PLAY` (normal gameplay), `STATE_TRANSITION` (scroll/fade between screens), `STATE_DEATH` (death pause + fade, then respawn).

### Shared types in game_config.h

`game_config.h` is the canonical source for compile-time constants (window size, tile size, combat tuning) and shared enums (`Direction`, `SoundID`). These were moved out of individual headers to break coupling — `Direction` is used by player, camera, and enemies; `SoundID` is used by game and sounds.

### Update loop order (STATE_PLAY)

`player_update` -> `enemies_update` -> `check_combat` -> death check -> low health beep -> `check_warp` -> `check_edge_transition`.

Combat collision (`check_combat` in game.c) handles sword-vs-enemy and enemy-contact-vs-player. It lives in game.c rather than a separate module because it's ~40 lines.

### Enemy system

Enemies use **vtable-style dispatch**: `EnemyDef` holds function pointers for `update` and `draw`, indexed by `EnemyType`. Adding a new enemy means writing its update/draw functions and adding one row to `enemy_defs[]` in `enemy.c`. No changes to the game loop needed.

Enemy files live in `src/enemy/`. Each enemy type is a separate .c file (slime.c, bat.c, charging_snake.c) with its AI implemented as a state machine using `EnemyState` and `state_timer`. The shared `Enemy` struct has a `velocity` field and `ai_timer` for type-specific scratch state.

Spawn data is parsed from screen metadata (`enemy: type col row` lines in .txt files) into `EnemySpawn` array in the `Screen` struct. Runtime `Enemy` instances live in `Game.enemies[]` and are re-spawned fresh on each screen load. Large slimes split into 2 small slimes on basic sword kill (sword_tier <= 1).

### Player combat

`PlayerState` includes `PSTATE_ATTACKING` (8-frame sword swing with cooldown) and `PSTATE_KNOCKBACK` (8-frame push on damage). Invulnerability lasts 60 frames with a visual flash. `player_sword_hitbox()` returns a TILE_SIZE rect one tile ahead in the facing direction. `player_take_damage()` applies damage + knockback + invulnerability.

### Transitions and screen loading

`trans_type` in Game struct tracks what kind of transition was started (scroll vs fade) because `camera.type` is reset to `TRANS_NONE` when the camera finishes — completion logic needs the original type to decide whether to swap `next_screen` (scroll) or not (fade, which loads directly into `current_screen` at midpoint). Cave enter/exit uses a return-stack that saves origin screen + tile position. Enemies are re-spawned after every screen load, including scroll transition completion.

### Debug overlay

`debug.c/.h` provides an F3-toggled overlay showing enemy count, alive/dead status, per-enemy type/state/position, player state, and HP. Yellow wireframe boxes mark enemy positions. Off by default.

## Coding Principles

- **C game programming best practices** — prefer stack/static allocation for fixed-size data, use heap when the size varies at runtime. Keep hot data contiguous, avoid unnecessary indirection.
- **KISS** — simplest thing that works. No clever patterns where a plain `if` does the job. But a plain `if` that must be copy-pasted into every new feature is not simple — it's a maintenance trap.
- **YAGNI** — don't build for hypothetical needs. No abstraction layers "for later."
- **DRY** — remove real duplication, not shape-similar code. Wrong abstraction costs more than repetition.
- **Locality of change** — adding a new entity, tile, or feature should require changes in as few files as possible. Prefer data-driven dispatch (flags, vtables) over centralized type switches when the set of types is expected to grow.
- **Dispatch strategy** — function pointers (vtable-style def tables) for large/growing type sets (enemies, projectiles). Switch statements for small/stable state machines (game state, player state).

When in doubt: for code one person owns and rarely changes, lean KISS. For interfaces many contributors touch, lean locality of change.

### Include discipline

Files in `src/enemy/` use relative paths for project headers (`../tilemap.h`, `../textures.h`) but standard include paths for vendor headers (`"raylib.h"`, `"raymath.h"`). Any file that calls raylib math functions like `Clamp` **must** include `"raymath.h"` — without it, MSVC silently treats the call as an implicit int function, passing float arguments through integer registers (garbage in, garbage out). The `/we4013` flag now catches this at compile time.

## Asset Pipeline

- **Sprites**: spritesheets drawn as single SVGs with all frames on a 64px grid, exported as one PNG via Inkscape (`"C:/Program Files/Inkscape/bin/inkscape.exe" player.svg -o player.png -w 256 -h 256` for a 4x4 sheet). One sheet per category (player, enemy type, tiles, items, etc.). Code indexes frames by row/column source rectangle. Store SVG and PNG in `src/assets/sprites/`.
- **Sounds**: generate with rfxgen (`"D:/rfxgen_v5.0_win_x64/rfxgen.exe" -g coin -o sound.wav`). Presets: coin, laser, explosion, powerup, hit, jump, blip. Store WAV in `src/assets/`.
- **Music**: OGG files in `src/assets/music/`. Loaded via `LoadMusicStream`, updated every frame. Composition pipeline: Python scripts in `tools/music/` use `midiutil` to generate MIDI -> FluidSynth renders with a soundfont to WAV -> ffmpeg converts to OGG. See `tools/music/compose_overworld.py` for the pattern.
- **Screen data**: plain text files in `src/assets/screens/` (overworld), `src/assets/dungeons/<n>/` (dungeons), `src/assets/caves/` (caves). Format: optional metadata lines (`warp:`, `enemy:`, `item:`, `#` comments), then 11 rows of 16 tile characters (W=wall, .=floor, ~=water, D=door, P=pushblock, S=stairs). Enemy spawn format: `enemy: type col row` where type is slime, bat, or snake.
- Sound and music loading is resilient — missing files are skipped.

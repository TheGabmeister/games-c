# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project

A 2D top-down action-adventure game inspired by The Legend of Zelda (1986), built in C with raylib. Phases 1 (Player and Tiles), 2 (World Navigation), 3 (Combat), and 4 (Projectiles and Items) are complete. Phase 5 (Dungeons) is next.

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

Single `Game` struct holds all state (defined in `game.h`). The main loop in `main.c` calls `game_init` -> `game_update`/`game_draw` per frame -> cleanup on exit. Game state machine: `STATE_PLAY` (normal gameplay), `STATE_TRANSITION` (scroll/fade between screens), `STATE_DEATH` (death pause + fade, then respawn), `STATE_PAUSE` (inventory/pause screen, game logic frozen).

### Shared types in game_config.h

`game_config.h` is the canonical source for compile-time constants (window size, tile size, combat tuning, projectile tuning) and shared enums/utilities (`Direction`, `SoundID`, `opposite_dir()`). These were moved out of individual headers to break coupling — `Direction` is used by player, camera, enemies, and projectiles; `SoundID` is used by game and sounds.

### Update loop order (STATE_PLAY)

`player_update` -> `enemies_update` -> `projectiles_update` -> `check_bomb_explosions` -> `vfx_update` -> `check_combat` -> `pickups_update` -> `check_pickups` -> death check -> low health beep -> `check_warp` -> `check_edge_transition`.

Combat collision (`check_combat` in game.c) handles sword-vs-enemy, player-projectile-vs-enemy (with slime splitting and boomerang stun), enemy-contact-vs-player, and enemy-projectile-vs-player (with shield blocking).

### Enemy system

Enemies use **vtable-style dispatch**: `EnemyDef` holds function pointers for `update` and `draw`, indexed by `EnemyType`. Adding a new enemy means writing its update/draw functions and adding one row to `enemy_defs[]` in `enemy.c`. No changes to the game loop needed.

Enemy files live in `src/enemy/`. Each enemy type is a separate .c file (slime.c, bat.c, charging_snake.c, rock_spitter.c, spear_thrower.c) with its AI implemented as a state machine using `EnemyState` and `state_timer`. The shared `Enemy` struct has a `velocity` field and `ai_timer` for type-specific scratch state.

The enemy `update` function signature includes projectile array access (`Projectile *projectiles, int *projectile_count`) so ranged enemies (rock_spitter, spear_thrower) can spawn projectiles. Non-ranged enemies ignore these params.

Spawn data is parsed from screen metadata (`enemy: type col row` lines in .txt files) into `EnemySpawn` array in the `Screen` struct. Runtime `Enemy` instances live in `Game.enemies[]` and are re-spawned fresh on each screen load. Supported enemy type names in screen files: `slime`, `bat`, `snake`, `rock_spitter`, `spear_thrower`.

### Projectile system

Projectiles use the same **vtable-style dispatch** as enemies: `ProjectileDef` holds function pointers for `update` and `draw`, indexed by `ProjectileType`. Types: `PROJ_ARROW`, `PROJ_BOOMERANG`, `PROJ_BOMB`, `PROJ_ROCK`, `PROJ_SPEAR`.

Arrow, rock, and spear share a generic `linear_update`/`linear_draw` in `projectile.c`. Boomerang has unique return-to-player homing. Bomb is stationary with a fuse timer — explosion damage is handled by `check_bomb_explosions` in game.c.

Each projectile has an `owner` field (`OWNER_PLAYER` or `OWNER_ENEMY`) that determines collision layers. Player projectiles hit enemies; enemy projectiles hit the player (with shield blocking based on facing direction and shield tier).

### Pickup and drop system

`pickup.h/c` manages a fixed array of pickups (max 16 per screen). Types: `PICKUP_RUPEE`, `PICKUP_HEART`, `PICKUP_BOMB`, `PICKUP_ARROW`. When enemies die, `try_spawn_drop()` in game.c rolls a random drop at the death position. The player collects pickups by walking over them. Pickups flash and despawn after 10 seconds. Cleared on screen transition.

### VFX system

`vfx.h/c` is a self-contained visual effects module with its own static array (max 16 effects). Fire-and-forget: call `vfx_spawn()`, and the effect updates and draws itself until it expires. Currently has `VFX_EXPLOSION` (expanding 3-layer fireball, 20 frames). Adding new effect types means adding a `VfxType` enum value and a draw function in vfx.c — nothing in game.h/.c changes.

### Player combat and items

`PlayerState` includes `PSTATE_ATTACKING` (8-frame sword swing with cooldown), `PSTATE_USING_ITEM` (12-frame item use with cooldown), and `PSTATE_KNOCKBACK` (8-frame push on damage). Invulnerability lasts 60 frames with a visual flash.

`player_update` takes projectile array params so the player can spawn projectiles on item use. Item dispatch is a switch on `inventory.equipped` in `try_use_item()` within player.c. Arrows cost 1 arrow (separate ammo, not rupees). Boomerang allows only one active at a time. Bombs cost 1 bomb, max one active.

### Inventory and pause screen

`inventory.h/c` owns the pause screen. `STATE_PAUSE` freezes game logic and draws a dimmed overlay with a 4x1 item grid (boomerang, bow, bomb, + empty slot). Cursor navigation + confirm equips an item. The `Inventory` struct in `player.h` tracks: `items` bitfield, `sword_tier`, `shield_tier`, `armor_tier`, `rupees`, `bombs`/`bomb_capacity`, `arrows`/`arrow_capacity`, `keys`, `equipped` item.

### Transitions and screen loading

`trans_type` in Game struct tracks what kind of transition was started (scroll vs fade) because `camera.type` is reset to `TRANS_NONE` when the camera finishes — completion logic needs the original type to decide whether to swap `next_screen` (scroll) or not (fade, which loads directly into `current_screen` at midpoint). Cave enter/exit uses a return-stack that saves origin screen + tile position. Enemies, projectiles, pickups, and VFX are all cleared on every screen load.

### Debug overlay

`debug.c/.h` provides an F3-toggled overlay showing enemy count, alive/dead status, per-enemy type/state/position, player state, HP, and active projectile count. Yellow wireframe boxes mark enemy positions. Off by default.

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

- **Sprites**: each entity/item gets its own SVG and PNG file on a 64px grid, exported via Inkscape. Single-frame sprites are 64x64; directional sprites (arrow, spear) are 256x64 (4 frames: S, N, E, W). Store SVG and PNG in `src/assets/sprites/`.
  - Enemies: `slime.png`, `bat.png`, `snake.png`, `rock_spitter.png` (placeholder), `spear_thrower.png` (placeholder)
  - Projectiles: `arrow.png` (4-dir), `boomerang.png`, `bomb.png`, `rock.png`, `spear.png` (4-dir)
  - Pickups: `pickup_rupee.png`, `pickup_heart.png`, `pickup_bomb.png`, `pickup_arrow.png`
  - All draw functions have colored-rectangle fallbacks when textures are missing.
- **Sounds**: generate with rfxgen (`"D:/rfxgen_v5.0_win_x64/rfxgen.exe" -g coin -o sound.wav`). Presets: coin, laser, explosion, powerup, hit, jump, blip. Store WAV in `src/assets/`.
- **Music**: OGG files in `src/assets/music/`. Loaded via `LoadMusicStream`, updated every frame. Composition pipeline: Python scripts in `tools/music/` use `midiutil` to generate MIDI -> FluidSynth renders with a soundfont to WAV -> ffmpeg converts to OGG. See `tools/music/compose_overworld.py` for the pattern.
- **Screen data**: plain text files in `src/assets/screens/` (overworld), `src/assets/dungeons/<n>/` (dungeons), `src/assets/caves/` (caves). Format: optional metadata lines (`warp:`, `enemy:`, `item:`, `#` comments), then 11 rows of 16 tile characters (W=wall, .=floor, ~=water, D=door, P=pushblock, S=stairs, B=bombable wall). Enemy spawn format: `enemy: type col row`.
- Sound and music loading is resilient — missing files are skipped.

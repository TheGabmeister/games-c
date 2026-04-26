# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project

Modernized Super Mario Bros clone written in C using raylib for rendering/input/audio. See `README.md` for project overview, controls, and game states. See `SPEC.md` for the full game specification including enemy behaviors, physics constants, and implementation phases.

## Build Commands

```bash
# Configure (from project root)
cmake -B build

# Build
cmake --build build

# Run (MSVC puts exe under Debug/; the exe uses its own directory as CWD via ChangeDirectory(GetApplicationDirectory()))
./build/super_mario_bros/Debug/super_mario_bros
```

No tests or linter — the build is the only verification step. The build copies `src/resources/` into the output directory automatically. New resource files (levels, sounds, sprites) must go in `src/resources/` to be included.

## Architecture

- **Language:** C (no C++), linked against raylib
- **Build system:** CMake, with raylib vendored under `vendor/raylib/`. Source files are recursively globbed from `src/`.
- **Window:** 1280x960, 64px tile grid (20x15 visible), 60 FPS. No framebuffer scaling — renders directly at window resolution.
- **Input:** keyboard (arrow keys, WASD, Space, Shift) and gamepad (left stick, D-pad, face buttons) simultaneously.

### Key design patterns

- One `Game` struct holds all state, passed by pointer.
- **Tagged entity array with vtables:** all dynamic objects (Mario, enemies, items, projectiles, debris) live in a flat `Entity entities[MAX_ENTITIES]` array. Each entity has a `type` tag and a pointer to a `const EntityVtab` (function pointers: `update`, `draw`, `touch`, `stomped`, `hit_by_fire`, `hit_by_shell`, `hit_by_star`, `bumped`, `kill`). The engine calls these callbacks — the entity type defines its own behavior.
- **Collision flags on entities:** `stompable`, `damages_mario`, `fire_immune`, `shell_killable`, `star_killable`, `destructible`, `self_moving`, `dead_falling`. Set at spawn time (or at runtime for `dead_falling`). The engine checks flags to decide *what kind* of interaction, then calls the vtable callback for the *type-specific response*.
- **game.c is the orchestrator:** runs update/collision loops and calls vtable callbacks. It does not contain entity-specific or tile-specific logic. Mario helpers (`mario_is_stomping`, `mario_take_damage`) live in `mario.c`. Firebar per-ball collision (`firebar_overlaps_entity`) lives in `firebar.c`.
- Each entity type has its own file(s) defining its vtable, spawn function, and behavior. Enemy files live under `src/enemies/`. All spawn functions share the signature `(Entity entities[], float x, float y, int extra)` and are registered in the `spawn_registry` table in `level.c`.
- `game_update()` dispatches to one handler function per game state. `game_draw()` has a similar per-state switch.
- Named constants for all tunable values live in `common.h` (`#define`). New magic numbers should be added there, not hardcoded inline.
- **Entities that manage their own movement** set `self_moving = true` at spawn time. The engine skips generic tile collision for these entities. When adding a new self-moving entity, set the flag in the spawn function — no changes to game.c needed.

### Level system

Levels are stored as `.txt` files in `src/resources/levels/` (e.g., `1-1.txt`, `1-2.txt`). The format is:

```
type overworld|underground|castle|athletic|underwater
bg R G B
bridge start_tx end_tx ty    # castle levels only

tiles
..GG..Q..BQB..    # one char per tile, 15 rows total
GGGGGGGGGGGGGG

blocks
21 9 mushroom     # tx ty content (coin/mushroom/starman/oneup/fire_flower/multi_coin)

spawns
22 12 goomba      # tx ty type [extra]
107 12 koopa red  # extra: "red" for red koopa, "cw" for clockwise firebar, pair_id for lifts
55 6 cheep 1      # extra: 0=gray slow, 1=red fast, 2=leaping

warps
16 11 1 2 142 11  # pipe_tx pipe_ty dest_world dest_sublevel dest_tx dest_ty
```

Tile characters: `.` empty, `G` ground, `B` brick, `Q` question, `U` used, `H` hard, `[` `]` pipe top, `{` `}` pipe body, `F` flagpole, `f` flagpole base, `=` bridge, `X` axe, `L` lava, `D` castle door, `I` invisible, `C` coral, `V` vine block, `T` bill blaster.

Spawn type names (used in `.txt` files): `goomba`, `koopa`, `piranha`, `firebar`, `podoboo`, `bowser`, `lift`, `paratroopa`, `springboard`, `blooper`, `cheep`, `hammer_bro`, `lakitu`, `spiny`, `buzzy`.

`level.c` contains the tile engine (collision, drawing, spawn activation), the `.txt` parser, and the `spawn_registry` table. Level data lives entirely in the `.txt` files — adding a new level requires no code changes. Adding a new enemy type requires: (1) its own `.c`/`.h` files with a `spawn_xxx(entities, x, y, extra)` function, (2) one entry in `spawn_registry`, (3) one entry in `parse_spawn_type`, (4) one `#include`.

Underwater levels (`type underwater`) modify Mario's physics in `mario.c`: reduced gravity, swim impulse on jump press, no running. The `V` (vine block) tile spawns a vine entity when hit from below and uses the `warps` section (keyed by the vine block's tile position) to define where the vine takes Mario.

### Game states

`STATE_TITLE`, `STATE_PLAYING`, `STATE_DYING`, `STATE_GAME_OVER`, `STATE_PAUSED`, `STATE_LEVEL_COMPLETE`, `STATE_CASTLE_COMPLETE`, `STATE_PIPE_TRANSITION`.

### Important caveats

- `DIR_NONE` is -1. Always guard with `if (dir != DIR_NONE)` before using direction as an array index.
- Dead-falling is signaled by setting `dead_falling = true` on the entity (alongside `state_val = 2` for backwards compat). The engine checks the flag, not the type — no type lists in game.c. Other entity types (piranha) use `state_val` for their own state machines and must not set `dead_falling`.
- Macro names must not collide with header include guards (e.g., use `PIRANHA_HEIGHT` not `PIRANHA_H`, `BOWSER_HEIGHT` not `BOWSER_H`).

## Asset Pipeline

Sprites are authored as SVGs and converted to PNGs using Inkscape:

```bash
"C:/Program Files/Inkscape/bin/inkscape.exe" input.svg -o output.png -w 64 -h 64
```

Store SVGs in `src/resources/svg/` and PNGs in `src/resources/`. Only PNGs are loaded at runtime.

## Sound Generation

Generate game sounds with rfxgen (by the raylib author).

```bash
# Available presets: coin, laser, explosion, powerup, hit, jump, blip
"D:/rfxgen_v5.0_win_x64/rfxgen.exe" -g coin -o sound.wav
```

Store WAV files in `src/resources/`.

## Coding principles

- **C game programming best practices** — prefer stack/static allocation for fixed-size data, use heap when the size varies at runtime (e.g. level tile grids). Keep hot data contiguous, avoid unnecessary indirection.
- **KISS** — simplest thing that works. No clever patterns where a plain `if` does the job. But a plain `if` that must be copy-pasted into every new feature is not simple — it's a maintenance trap.
- **YAGNI** — don't build for hypothetical needs. No abstraction layers "for later."
- **DRY** — remove real duplication, not shape-similar code. Wrong abstraction costs more than repetition.
- **Locality of change** — adding a new entity, tile, or feature should require changes in as few files as possible. Prefer data-driven dispatch (flags, vtables) over centralized type switches when the set of types is expected to grow. If a new enemy requires editing the orchestrator, the abstraction is missing.

When in doubt: for code one person owns and rarely changes, lean KISS. For interfaces many contributors touch, lean locality of change.

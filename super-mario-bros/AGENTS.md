# AGENTS.md

Guidance for AI coding agents working in this repository.

## Project

Modernized Super Mario Bros clone written in C using raylib for rendering,
input, and audio. Gameplay follows classic SMB mechanics, while visuals use
modern PNG sprites created from SVGs via Inkscape.

Use `README.md` for the project overview and `SPEC.md` for detailed mechanics,
entity design, and implementation order. Treat `SPEC.md` as the target design,
but verify the current code before assuming every planned system exists.

## Build Commands

```bash
# Configure from the project root
cmake -B build

# Build
cmake --build build

# Run on MSVC / Visual Studio generators
./build/super_mario_bros/Debug/super_mario_bros
```

The build copies `src/resources/` into the output directory automatically.

## Architecture

- Language: C only. Do not introduce C++.
- Build system: CMake.
- Rendering/input/audio: raylib, vendored under `vendor/raylib/`.
- Source layout: all `.c` and `.h` files live under `src/` and are recursively globbed.
- Assets: sprites (PNG) and sounds (WAV) live in `src/resources/`. SVG sources live in `src/resources/svg/`.
- Levels: file-based text maps live in `src/resources/levels/`.
- Window: 1280x960, 64px tile grid (20x15 visible), 60 FPS. No framebuffer scaling; render directly at window resolution.
- Input: keyboard (arrow keys, WASD, Space, Shift) and gamepad (left stick, D-pad, face buttons) simultaneously.
- Visual style: modernized clean sprites with particle effects. Not pixel-art retro.

## Code Structure

- `common.h`: shared constants, colors, types (`EntityType`, `GameState`, `Direction`).
- `entity.c` / `entity.h`: `Entity`, `EntityVtab`, generic allocation/deactivation, AABB overlap.
- `mario.c` / `mario.h`: Mario vtable, input, physics, damage, spawn.
- `enemies/*.c/h`: one file per enemy or object family, each defining vtables + spawn functions.
- `items.c` / `items.h`: coins, powerups, fireballs, popups, debris, and balance lifts.
- `level.c` / `level.h`: tile grid, text level loading, tile types, tile collision, entity activation, head-bump behavior.
- `camera.c` / `camera.h`: camera follow logic, threshold, clamping.
- `particles.c` / `particles.h`: fixed-size particle effects.
- `sprites.c` / `sprites.h`: texture loading and sprite lookup.
- `sounds.c` / `sounds.h`: WAV loading and sound playback.
- `game.c` / `game.h`: `Game` state machine, top-level update/draw orchestration, HUD.
- `main.c`: window/audio initialization, main loop, cleanup.

## Design Patterns

- One `Game` struct owns top-level state and is passed by pointer. Prefer stack/static allocation for fixed-size data; use heap when size varies at runtime, such as level tile grids.
- Dynamic objects live in `Entity entities[MAX_ENTITIES]`. Each entity has a `type` tag and a pointer to a `static const EntityVtab` with callbacks (`update`, `draw`, `touch`, `stomped`, `hit_by_fire`, `hit_by_shell`, `hit_by_star`, `bumped`, `kill`).
- Collision flags (`stompable`, `damages_mario`, `fire_immune`, `shell_killable`, `star_killable`, `destructible`) are set at spawn time. The engine should use flags for generic decisions, then call vtable callbacks for type-specific behavior.
- `game.c` should remain an orchestrator: run loops, detect overlaps, call callbacks, manage game state. Avoid adding new enemy-specific or item-specific logic there.
- Each entity type should own its behavior in its own file. Adding an enemy should normally mean adding a vtable + spawn function and registering spawn parsing/activation, not expanding collision special cases in `game.c`.
- Tile interaction is currently implemented in `level_handle_head_bump()` rather than a separate `blocks.c` handler table. If this grows, prefer introducing the handler table described in `SPEC.md` instead of adding more branching to `level.c`.
- Side-scrolling camera follows Mario and never scrolls backward. Mario should not move left past the camera edge.

## Current Guardrails

- Do not silently rely on `level_load()` falling back to `1-1` for missing future levels. If adding or advancing past implemented content, make progression explicit.
- Keep HUD drawing after level/entities so tiles and entities cannot paint over it.
- If adding Phase 6 Bullet Bills or Bill Blasters, avoid another hardcoded projectile exception list in `game.c`; prefer flags, callbacks, or a small generic collision category.
- Keep level maps at 15 tile rows unless deliberately changing the visible-area model.
- When adding heap ownership, pair it with a clear `level_free()`/cleanup path. Be careful around `game_init()` because it clears state.
- Do not edit vendored raylib unless explicitly requested.

## Coding Principles

- KISS: prefer the simplest clear implementation. Plain control flow is usually better than clever abstraction.
- YAGNI: do not add configuration, extension points, or systems for hypothetical future needs.
- DRY: remove real duplication, but avoid extracting code just because it has a similar shape.
- Keep changes focused on the requested behavior.
- Match the existing C style and project structure before adding new patterns.

When in doubt, lean KISS over DRY. A small amount of readable repetition is better than the wrong shared helper.

## Asset Pipeline

Sprites are authored as SVGs and converted to PNGs using Inkscape:

```bash
# Convert SVG to PNG
"C:/Program Files/Inkscape/bin/inkscape.exe" input.svg -o output.png -w 64 -h 64
```

Commit both SVG sources and generated PNG files.

## Resources

Sound effects are WAV files in `src/resources/`, generated with rfxgen.

```bash
# Available presets: coin, laser, explosion, powerup, hit, jump, blip
"D:/rfxgen_v5.0_win_x64/rfxgen.exe" -g coin -o src/resources/sound.wav
```

Commit generated WAV files. Do not add rfxgen to the game build.

## Verification

- Run `cmake --build build` after code changes when feasible.
- If audio code changes, ensure `InitAudioDevice()` and `CloseAudioDevice()` remain balanced.
- For level or progression changes, run through the affected transition manually when feasible.

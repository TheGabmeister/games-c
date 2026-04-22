# AGENTS.md

Guidance for AI coding agents working in this repository.

## Project

Modernized Super Mario Bros clone written in C using raylib for rendering, input, and audio. The gameplay follows classic SMB mechanics, while visuals use modern PNG sprites created from SVGs via Inkscape.

Use `README.md` for the project overview and `SPEC.md` for detailed game mechanics, entity system design, and implementation order.

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
- Window: 1200x900, 16px tile grid, 60 FPS.
- Input: keyboard (arrow keys, WASD, Space, Shift) and gamepad (left stick, D-pad, face buttons) simultaneously.
- Visual style: modernized clean sprites with particle effects. Not pixel-art retro.

## Code Structure

- `common.h`: shared constants, colors, types (EntityType, GameState, Direction).
- `entity.c` / `entity.h`: Entity struct, EntityVtab struct, collision flag helpers, generic spawn/kill.
- `mario.c` / `mario.h`: Mario vtable, input, physics, state transitions, spawn.
- `enemies/*.c/h`: one file per enemy type (goomba, koopa, lakitu, bowser, etc.), each defining a vtable + spawn function.
- `items.c` / `items.h`: coin, mushroom, fire flower, starman, 1-Up vtables + spawn.
- `blocks.c` / `blocks.h`: tile handler table for block interactions (brick, question block, etc.).
- `level.c` / `level.h`: tile grid, tile types, level data, tile collision, entity activation.
- `camera.c` / `camera.h`: camera follow logic, threshold, clamping.
- `particles.c` / `particles.h`: fixed-size particle effects.
- `game.c` / `game.h`: Game struct, state machine, collision loops, HUD, orchestration.
- `main.c`: window/audio initialization, main loop, cleanup.

## Design Patterns

- One `Game` struct owns top-level state and is passed by pointer. Prefer stack/static allocation for fixed-size data; use heap when the size varies at runtime (e.g. level tile grids).
- **Tagged entity array with vtables:** all dynamic objects live in `Entity entities[MAX_ENTITIES]`. Each entity has a `type` tag and a pointer to a `static const EntityVtab` with function-pointer callbacks (`update`, `draw`, `touch`, `stomped`, `hit_by_fire`, `hit_by_shell`, `hit_by_star`, `bumped`, `kill`). The engine calls these — the entity defines its own response.
- **Collision flags:** `stompable`, `damages_mario`, `fire_immune`, `shell_killable`, `star_killable`, `destructible` — set at spawn time. The engine checks flags for the generic decision, then calls the vtable callback for the type-specific response.
- **Tile handler table:** block/tile interactions dispatch via function pointer table indexed by tile type. Each tile type defines its own handler.
- `game.c` is the orchestrator only: runs loops, detects overlaps, calls callbacks. It does not contain entity-specific or tile-specific interaction logic.
- Each entity type owns its behavior in its own file. Adding a new enemy = new vtable + spawn function, no edits to `game.c`.
- Side-scrolling camera follows Mario, never scrolls backward. Levels are tile-based (16x16 pixels).
- See `SPEC.md` for full vtable definitions, update/draw flow, and collision system.

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
"C:/Program Files/Inkscape/bin/inkscape.exe" input.svg -o output.png -w 16 -h 16
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
- If audio code changes, ensure `InitAudioDevice()` and `CloseAudioDevice()` still remain balanced.
- Avoid editing vendored raylib unless explicitly requested.

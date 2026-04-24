# AGENTS.md

## Project

Defender (1981 arcade) clone written in C using raylib for rendering/input/audio.

## Build Commands

```bash
# Configure (from project root)
cmake -B build

# Build
cmake --build build

# Run (executable outputs to build/defender/Debug/ on MSVC)
./build/defender/Debug/defender.exe
```

The build copies `src/resources/` into the output directory automatically. CMake auto-discovers all `.c`/`.h` files in `src/` via `GLOB_RECURSE` -- no CMakeLists.txt changes needed when adding new source files.

## Architecture

- **Language:** C (no C++), linked against raylib
- **Build system:** CMake, with raylib vendored under `vendor/raylib/`
- **Source layout:** All `.c` and `.h` files live under `src/` (flat, no subdirectories). Game assets go in `src/resources/`.

### Central header: `game.h`

Every gameplay module includes `game.h`. It contains shared constants, enums (`GameState`, `Facing`, `EnemyKind`, `HumanoidState`, `SoundID`), struct definitions (`Game`, `PlayerShip`, `Laser`, `EnemyBullet`, `Particle`, `Terrain`, `Humanoid`, `Enemy`), inline world wrapping helpers (`wrap_x`, `wrapped_delta`), and subsystem function declarations.

The `Game` struct is the single top-level state container -- passed by pointer to all subsystems. It owns player state, fixed-size entity arrays, terrain samples, camera/radar state, score/lives/wave state, and loaded sounds.

### Module boundaries

- `main.c` owns the raylib window/audio lifecycle and calls `game_init`, `game_update`, and `game_draw`.
- `game.c` orchestrates the state machine and decides which subsystem updates run for each `GameState`.
- `world.c` owns common math helpers, score bookkeeping, particles, terrain generation, player/wave resets, and wave spawning.
- `entities.c` owns gameplay simulation: player input, lasers, enemy bullets, humanoid state, enemy AI, collisions, deaths, smart bombs, hyperspace, and wave completion checks.
- `render.c` owns all drawing for the world, radar, HUD, title/game-over overlays, particles, and entities.
- `sounds.c` / `sounds.h` own resilient sound loading, unloading, and playback.

Keep new gameplay functions close to the state they update. Prefer `static` helpers inside the owning `.c` file unless another module genuinely needs to call them.

### World model

The game world is wider than the window (`WORLD_WIDTH`) and wraps horizontally. Store x positions in world space, normalize them with `wrap_x`, compare wrapped distances with `wrapped_delta`, and convert to screen x with `world_to_screen_x` when drawing.

Terrain is sampled in `Terrain.height[TERRAIN_SAMPLES]`; use `terrain_height_at` instead of indexing samples directly for gameplay decisions.

## Coding principles

- **C game programming best practices** -- prefer stack/static allocation for fixed-size data, use heap when the size varies at runtime. Keep hot data contiguous, avoid unnecessary indirection.
- **KISS** -- simplest thing that works. No clever patterns where a plain `if` does the job.
- **YAGNI** -- don't build for hypothetical needs. No abstraction layers "for later."
- **DRY** -- remove real duplication, not shape-similar code. Wrong abstraction costs more than repetition.

When in doubt, lean KISS over DRY.

## Sprite Generation

Workflow: write SVG markup, then convert to PNG with Inkscape.

```bash
inkscape input.svg -o output.png -w 20 -h 20
```

Store both SVGs and PNGs in `src/resources/`. Cell size is 20px.

## Sound Generation

Generate game sounds with rfxgen (by the raylib author).

```bash
# Available presets: coin, laser, explosion, powerup, hit, jump, blip
"D:/rfxgen_v5.0_win_x64/rfxgen.exe" -g coin -o sound.wav
```

Store WAV files in `src/resources/`. Sound loading is resilient -- missing files are skipped, present files play normally.

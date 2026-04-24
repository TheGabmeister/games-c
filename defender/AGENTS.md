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

Every module includes `game.h`. It contains all `#define` constants, enums (`GameState`, `Direction`, `SoundID`), struct definitions (`Game`, `Player`, `Dart`, `Segment`, `Spider`, `Flea`, `Scorpion`, `MushroomGrid`), and inline coordinate conversion helpers (`col_to_px`, `row_to_px`, `px_to_col`, `px_to_row`). The `Game` struct is the single top-level state container -- passed by pointer to all subsystems.

### Module dependency graph

`game.c` is the orchestrator: it owns the state machine, calls all subsystem update/draw functions, and implements all collision detection as static helpers.

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

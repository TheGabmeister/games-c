# AGENTS.md

Guidance for AI coding agents working in this repository.

## Project

Modernized Pac-Man clone written in C using raylib for rendering, input, and audio. The gameplay follows classic Pac-Man mechanics, while visuals are procedural with a neon/modern style.

Use `README.md` for the project overview. There is currently no `SPEC.md` in the repo, so prefer the existing code and README as the source of truth.

## Build Commands

```bash
# Configure from the project root
cmake -B build

# Build
cmake --build build

# Run on MSVC / Visual Studio generators
./build/pac_man/Debug/pac_man
```

The build copies `src/resources/` into the output directory automatically.

## Architecture

- Language: C only. Do not introduce C++.
- Build system: CMake.
- Rendering/input/audio: raylib, vendored under `vendor/raylib/`.
- Source layout: all `.c` and `.h` files live under `src/` and are recursively globbed.
- Assets: game assets live in `src/resources/`.
- Window: 800x1000, 24px tile grid (28x36), 60 FPS.
- Input: keyboard (arrow keys, WASD) and gamepad (left stick, D-pad) simultaneously.
- Visual style: modernized neon look with rounded/glowing walls, particles, and procedural graphics. No sprite sheets are currently used.

## Code Structure

- `common.h`: shared constants, colors, types, direction helpers, speed/level tier helpers, and maze helper declarations.
- `maze.c` / `maze.h`: 28x36 tile grid, direction vectors, Cruise Elroy thresholds, tunnel/walkability helpers.
- `pacman.c` / `pacman.h`: Pac-Man state, input, movement, cornering, animation, and drawing.
- `ghost.c` / `ghost.h`: ghost state, scatter/chase/frightened/eaten behavior, AI targeting, pathfinding, house movement, and drawing.
- `fruit.c` / `fruit.h`: fruit spawning, scoring, timers, and drawing.
- `particles.c` / `particles.h`: fixed-size particle effects.
- `game.c` / `game.h`: top-level game state, state machine, scoring, dots, sounds, HUD, collision, timers, and orchestration.
- `main.c`: window/audio initialization, main loop, cleanup.

## Design Patterns

- One `Game` struct owns top-level state and is passed by pointer.
- Prefer fixed-size arrays and stack/file-scoped state over heap allocation.
- Keep module ownership clear: `game.c` orchestrates, but movement/AI/drawing details live in their modules.
- Tile coordinates are integer grid positions; entity positions are floating-point pixels relative to the maze origin.
- Maze data is read-only. Mutable dot state lives in `Game.dot_eaten`.
- Use the three walkability helpers intentionally:
  - `maze_is_walkable`
  - `maze_is_walkable_pacman`
  - `maze_is_walkable_ghost`

## Coding Principles

- KISS: prefer the simplest clear implementation. Plain control flow is usually better than clever abstraction.
- YAGNI: do not add configuration, extension points, or systems for hypothetical future needs.
- DRY: remove real duplication, but avoid extracting code just because it has a similar shape.
- Keep changes focused on the requested behavior.
- Match the existing C style and project structure before adding new patterns.

When in doubt, lean KISS over DRY. A small amount of readable repetition is better than the wrong shared helper.

## Resources

Sound effects are WAV files in `src/resources/`, generated with rfxgen.

```bash
# Available presets: coin, laser, explosion, powerup, hit, jump, blip
"D:/rfxgen_v5.0_win_x64/rfxgen.exe" -g coin -o src/resources/sound.wav
```

Commit generated WAV files. Do not add rfxgen to the game build.

## Verification

- Run `cmake --build build` after code changes when feasible.
- If audio code changes, ensure `InitAudioDevice()`, `game_load_sounds()`, `game_unload_sounds()`, and `CloseAudioDevice()` still remain balanced.
- Avoid editing vendored raylib unless explicitly requested.

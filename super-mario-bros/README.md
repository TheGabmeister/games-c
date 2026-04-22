# Super Mario Bros

A modernized Super Mario Bros clone written in C using [raylib](https://www.raylib.com/) for rendering, input, and audio.

Features classic SMB gameplay — running, jumping, stomping enemies, breaking blocks, collecting coins and power-ups, progressing through levels — rendered with a modern visual style using PNG sprites created from SVGs via Inkscape.

## Screenshot

*(Add a screenshot here)*

## Building

Requires CMake and a C compiler (tested with MSVC on Windows).

```bash
cmake -B build
cmake --build build
```

The build copies `src/resources/` into the output directory automatically. raylib is vendored under `vendor/raylib/` and built alongside the project.

## Running

```bash
# MSVC puts the executable under Debug/
./build/super_mario_bros/Debug/super_mario_bros
```

## Controls

| Action | Keyboard | Gamepad |
|--------|----------|---------|
| Move | Arrow keys / A,D | Left stick / D-pad |
| Jump | Space / W / Up | A button |
| Run / Fire | Left Shift | X button |
| Pause | Esc / P | Start |
| Start game | Enter | Any face button |

All input methods work simultaneously.

## Project Structure

```
src/
  common.h        Shared types, constants, colors
  particles.h/c   Particle effects system
  game.h/c        Top-level game state, orchestration, HUD
  main.c          Entry point: window/audio init, game loop
  resources/      Sprites (PNG) and sound effects (WAV)
vendor/
  raylib/         Vendored raylib library
```

## Architecture

- **One `Game` struct** holds all state and is passed by pointer. No heap allocation.
- **Modular design:** each module owns its struct and logic. `game.c` orchestrates them, passing only the fields each module needs.
- **Side-scrolling camera:** the camera follows Mario horizontally; levels scroll left-to-right.
- **Tile-based levels:** 16x16 pixel tiles. Collision detection uses tile grid lookups.
- **Sprite-based graphics:** PNGs created from SVGs using Inkscape. Sound effects are WAV files generated with rfxgen.

## Asset Pipeline

Sprites are authored as SVGs and converted to PNGs using Inkscape:

```bash
# Convert SVG to PNG (example: 64x64 tile)
"C:/Program Files/Inkscape/bin/inkscape.exe" input.svg -o output.png -w 64 -h 64
```

Store SVGs in `src/resources/svg/` and PNGs in `src/resources/`.

## License

This project is for personal/educational use.

# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project overview

Lode Runner (1983) recreation in C11 with raylib. 1200x900 window, 60 fps, 30x22 tile grid at 36px per cell. Full design spec is in `SPEC.md`.


## Coding principles

- KISS over DRY. Prefer stack/static allocation.
- Constants live in `game_config.h`. Tile size is 36px (not the template default 20px).
- Asset paths are relative to the output directory: `assets/coin.wav`, `assets/levels/level01.txt`.
- Sprites: SVG → PNG via `inkscape input.svg -o output.png -w 36 -h 36`.
- Sounds: WAV via rfxgen (`"D:/rfxgen_v5.0_win_x64/rfxgen.exe" -g coin -o sound.wav`), or generated procedurally at runtime.

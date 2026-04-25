# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

A modernized twin-stick arena shooter inspired by Robotron 2084, written in C on top of bundled raylib. Original recreation — do not use original ROM, ripped sprites, cabinet art, or sampled arcade audio.

## Build

```bash
cmake --build build
```

If `build/` is missing or stale, configure first:

```bash
cmake -S . -B build
cmake --build build
```

The executable lands in `build/robotron/Debug/` (default Visual Studio generator). Assets are copied to `<exe-dir>/assets/` post-build, so runtime paths like `assets/player.png` are relative to the executable directory.

There are no tests, no linter, and no separate run target — launch the built `.exe` directly.

## Architecture

`Game` (in [src/game.h](src/game.h)) is the single owner of fixed-size arrays for enemies, humans, bullets, projectiles, electrodes, particles, and floating score text. The codebase **deliberately avoids dynamic allocation and any generic ECS** — keep hot data contiguous and fixed-capacity. Do not replace `Game`'s arrays with a registry or ECS.

Update flow lives in [src/world.c](src/world.c), which sequences the playfield each frame: player/bullets → humans → enemy families → projectiles → effects → collision phases → wave-clear checks → draw. The specialized `world_*.c` files group behavior by concern but never take ownership away from `Game`:

- [world_entities.c](src/world_entities.c) — enemy + human + spawner update and AI (Grunt, Hulk, Spheroid, Enforcer, Quark, Tank, Brain, Prog), plus wave construction.
- [world_projectiles.c](src/world_projectiles.c) — bullets, sparks, bouncing shells, homing cruise missiles.
- [world_collisions.c](src/world_collisions.c) — circle-vs-circle resolution and ordering.
- [world_effects.c](src/world_effects.c) — particles, screen shake, floating score text.
- [world_draw.c](src/world_draw.c) — sprite + primitive rendering for the playfield.
- [world_internal.h](src/world_internal.h) — declarations shared between `world_*.c` files.

[main.c](src/main.c) does raylib init, main loop, shutdown. [game.c](src/game.c) owns the top-level state machine (title, wave intro, playing, paused, player dead, game over), HUD, and overlays.

**All compile-time tuning (speeds, fire rates, max counts, scoring, spawn timers, presentation timings) lives in [src/game_config.h](src/game_config.h).** Adjust numbers there, not inline.

Enemy-family behavior stays explicit in the relevant update/draw loops. When a policy is shared across families (e.g. whether the family counts for wave clear, kills the player on contact, follows the normal bullet-kill path), centralize it in a small helper rather than duplicating per-caller.

## Assets

PNG sprites are required: missing/invalid registered textures in [src/textures.c](src/textures.c) cause loud failure. WAV sounds are optional: missing entries from [src/sounds.c](src/sounds.c) are skipped silently.

Sprites are authored as SVG and converted to PNG via Inkscape; both source SVG and generated PNG live in `src/assets/`. Cell size is 20px. On Windows, Inkscape is at `C:/Program Files/Inkscape/bin/inkscape.com`:

```powershell
& "C:/Program Files/Inkscape/bin/inkscape.com" --export-type=png --export-filename="src/assets/player.png" --export-width=20 --export-height=20 "src/assets/player.svg"
```

Sounds are generated with rfxgen (by raylib's author):

```bash
"D:/rfxgen_v5.0_win_x64/rfxgen.exe" -g coin -o sound.wav
```

rfxgen may not write to nested paths — generate in repo root and copy into `src/assets/`. Repeated-fire sounds (`player_shoot.wav`) must be low-fatigue: prefer soft/low presets, avoid harsh high-pitched lasers.

## Coding principles

- Prefer stack/static allocation for fixed-size data; heap only when size truly varies at runtime.
- KISS over DRY when in doubt — wrong abstraction costs more than repetition.
- YAGNI — no abstraction layers "for later."
- Build with `cmake --build build` after code changes; keep each milestone playable.

## Further reading

- [README.md](README.md) — full project overview, gameplay reference (wave clear rules, rescue ladder, lives, brain waves, electrodes, hulks), and a "Where To Edit" map.
- [AGENTS.md](AGENTS.md) — extended notes on build, asset pipeline, and coding principles.

# Robotron 2084 (raylib C recreation)

A modernized twin-stick arena shooter inspired by the 1982 arcade game, built
in C on top of raylib. The player fights waves of robots, rescues humans for
escalating bonuses, and survives across a single non-scrolling screen.

This is an original recreation. No original ROM, ripped sprites, cabinet art,
or sampled audio is used.

- Resolution: `1200x900` at `60 FPS`.
- Controls: `WASD` to move, arrow keys to fire (8-direction), `Enter` to
  start/restart, `P` to pause, `Esc` to quit. Gamepad supported (left stick
  move, right stick fire).

## Build

The project uses CMake and bundles raylib under `vendor/raylib/`.

```bash
cmake -S . -B build      # only needed once, or after CMake changes
cmake --build build
```

The executable is emitted under `build/template/Debug/` (default Visual Studio
generator). Assets are copied next to the binary as `assets/...` by a
post-build step in [CMakeLists.txt](CMakeLists.txt).

## Project Layout

```
src/
  main.c              raylib init, main loop, shutdown
  game.h / game.c     shared constants, enums, structs, top-level state
                      transitions, HUD, overlays
  world.h / world.c   playfield simulation entry points
  world_internal.h    shared types between world_*.c files
  world_entities.c    enemy / human / spawner update + AI
  world_projectiles.c bullets, sparks, shells, cruise missiles
  world_collisions.c  circle-vs-circle resolution and ordering
  world_effects.c     particles, screen shake, floating score text
  world_draw.c        sprite + primitive rendering for the playfield
  textures.h / .c     optional PNG sprite loading (skips missing files)
  sounds.h / .c       optional WAV sound loading (skips missing files)
  assets/             source SVGs, generated PNGs, generated WAVs
vendor/raylib/        bundled raylib source
CMakeLists.txt        build + asset copy
AGENTS.md             agent-facing build/coding notes
```

`Game` (in [src/game.h](src/game.h)) owns fixed-size arrays for enemies,
humans, bullets, projectiles, electrodes, particles, and floating score text.
The code prefers contiguous, fixed-capacity data over dynamic allocation or a
generic ECS.

## Where to Edit

- **Tuning numbers** (player speed, fire rate, bullet lifetime, max counts,
  scoring values, extra-life threshold): constants at the top of
  [src/game.h](src/game.h).
- **Game state machine** (title, wave intro, playing, paused, player dead,
  game over): [src/game.c](src/game.c).
- **Wave contents and difficulty scaling**: wave-building functions in
  [src/world.c](src/world.c) / [src/world_entities.c](src/world_entities.c).
- **Enemy AI** (Grunt, Hulk, Spheroid, Enforcer, Quark, Tank, Brain, Prog):
  [src/world_entities.c](src/world_entities.c).
- **Bullets and enemy projectiles** (sparks, bouncing shells, homing cruise
  missiles): [src/world_projectiles.c](src/world_projectiles.c).
- **Collision order and rules**: [src/world_collisions.c](src/world_collisions.c).
- **Particles, screen shake, hit flash, floating score text**:
  [src/world_effects.c](src/world_effects.c).
- **Sprite/primitive rendering**: [src/world_draw.c](src/world_draw.c).
- **Texture / sound IDs** and load lists:
  [src/textures.c](src/textures.c) and [src/sounds.c](src/sounds.c).

Both texture and sound loading are resilient — missing assets are skipped and
gameplay falls back to colored primitives or silence, so it is safe to remove
or add asset files without breaking the build.

## Asset Pipeline

Sprites are authored as 20px SVGs in [src/assets/](src/assets/) and exported
to PNG with Inkscape:

```bash
inkscape src/assets/player.svg -o src/assets/player.png -w 20 -h 20
```

On Windows, Inkscape may need its full path:

```powershell
& "C:/Program Files/Inkscape/bin/inkscape.com" `
  --export-type=png --export-filename="src/assets/player.png" `
  --export-width=20 --export-height=20 "src/assets/player.svg"
```

Sounds are generated with rfxgen (by the raylib author) into the same folder:

```bash
"D:/rfxgen_v5.0_win_x64/rfxgen.exe" -g laser -o src/assets/player_shoot.wav
```

Available presets: `coin`, `laser`, `explosion`, `powerup`, `hit`, `jump`,
`blip`. Keep repeated-fire sounds soft to avoid fatigue.

After adding or replacing an asset, register its ID in
[src/textures.c](src/textures.c) or [src/sounds.c](src/sounds.c) and rebuild —
the post-build step copies `src/assets/` next to the executable automatically.

## Gameplay Reference

- **Wave clear**: all non-Hulk enemies destroyed. Remaining Hulks, humans,
  electrodes, bullets, and hostile projectiles are removed during the
  transition.
- **Human rescue ladder**: `1000`, `2000`, `3000`, `4000`, then `5000` each.
  Resets on new wave and on player death.
- **Lives**: start at `3`, extra life every `25000` points.
- **Brain waves**: every fifth wave. Brains convert humans to Progs and fire
  homing cruise missiles.
- **Electrodes**: kill the player and Grunts on contact, can be shot for `0`
  points.
- **Hulks**: invulnerable, hunt humans, knocked back / slowed by bullets, do
  not block wave clear.

The original design notes (research summary, full enemy roster, scoring
table, wave templates, milestones) lived in `SPEC.md` and were the source for
this README.

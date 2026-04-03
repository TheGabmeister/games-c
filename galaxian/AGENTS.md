# AGENTS.md

This file provides guidance to coding agents working in this repository.

## Build Commands

```powershell
# Configure (Visual Studio 18 2026 generator, x64)
cmake -S . -B build-codex -G "Visual Studio 18 2026" -A x64 -Wno-dev

# Build
cmake --build build-codex --config Debug

# Run
.\build-codex\Debug\galaxian.exe
```

Dependencies (`SDL3`, `SDL3_image`, `SDL3_mixer`, `SDL3_ttf`) live in `vendor/`
and are built via `add_subdirectory()`.

When adding new `.c` files, add them to the `add_executable()` call in
`CMakeLists.txt`.

## Project Overview

This repository is no longer a generic SDL starter. It is a Galaxian remake in
C17 using SDL3 with a small engine-style core plus game-specific modules.

Current gameplay direction:

- Portrait playfield at `600 x 800`
- Primitive-rendered ships, bullets, particles, and background effects
- Session-only high score tracking
- Menu, gameplay, and game-over states are implemented
- Audio is initialized by the app shell, but gameplay currently does not depend
  on authored audio assets

The gameplay spec lives in `SPEC.md`. If code and spec diverge, update both or
leave a note explaining the intentional difference.

## Architecture

### `src/main.c`

Application entry point.

- Creates the window using `SCREEN_W` and `SCREEN_H` from `galaxian.h`
- Initializes audio and enables alpha blending on the renderer
- Registers `STATE_MENU`, `STATE_GAMEPLAY`, and `STATE_GAME_OVER`
- Starts at `STATE_MENU`
- Owns the main loop and shutdown order

Main loop order:

1. `engine_begin_frame()`
2. `SDL_PollEvent()` + `engine_process_event()`
3. `get_deltatime()` -> `game_state_update(dt)`
4. Clear -> `game_state_draw()` -> `SDL_RenderPresent()`

Shutdown order:

1. `game_state_shutdown()`
2. `res_free_all()`
3. `audio_shutdown()`
4. `close_window()`

### `src/platform.c` / `src/platform.h`

SDL-facing platform wrapper.

- Window lifecycle: `init_window()`, `close_window()`, `is_window_ready()`,
  `window_should_close()`, `request_close()`
- Frame timing: `get_deltatime()`, `get_fps()`
- Renderer access: `get_renderer()`
- Input polling with edge detection:
  `is_key_pressed()` vs `is_key_down()`,
  `is_mouse_button_pressed()` vs `is_mouse_button_down()`
- Shared utility types and key/button enums live here

Keep SDL runtime state internal to this module.

### `src/game_state.c` / `src/game_state.h`

Simple state/scene system.

- Register states with `game_state_register(STATE_ID, callbacks)`
- Each state may provide `init`, `update(dt)`, `draw`, and `cleanup`
- Switch states with `game_state_switch()`
- Current enum includes `STATE_MENU`, `STATE_GAMEPLAY`, `STATE_PAUSE`, and
  `STATE_GAME_OVER`

`STATE_PAUSE` is reserved but not part of the current gameplay loop.

### `src/galaxian.c` / `src/galaxian.h`

Shared gameplay definitions and reusable Galaxian-specific helpers.

- Screen, HUD, player, bullet, formation, and timing constants
- Shared structs: `Player`, `Enemy`, `Bullet`, `Particle`, `Star`, `DiffParams`
- Session-level score helpers:
  `gx_high_score()`, `gx_set_high_score()`, `gx_last_score()`,
  `gx_set_last_score()`, `gx_is_new_high()`, `gx_set_new_high()`
- Difficulty tuning:
  `gx_difficulty()`, `gx_first_attack_delay()`, `gx_sway_speed()`
- Formation setup and enemy selection:
  `gx_formation_setup()`, `gx_pick_solo_diver()`, `gx_pick_convoy()`,
  `gx_convoy_score()`
- Path helpers:
  `bezier_eval()`, `gx_dive_path()`, `gx_return_path()`
- Starfield update/draw helpers
- State callback declarations for menu, gameplay, and game over

Treat `galaxian.h` as the gameplay contract. Reuse its constants instead of
duplicating numbers in state files.

### `src/state_menu.c`

Title screen and attract-mode presentation.

- Initializes fonts and starfield state
- Shows title, session high score, controls hint, and animated attract enemies
- `Enter` starts gameplay
- `Escape` requests app close through `request_close()`

### `src/state_gameplay.c`

Primary gameplay state.

- Owns run state: player, enemies, bullets, particles, stars, score, lives,
  stage progression, convoy tracking, stage intro/clear timers, and shake
- Uses `galaxian.c` helpers for formation setup, path creation, scoring, and
  difficulty scaling
- Handles player movement, firing, collisions, stage clear, extra life, and
  swarm mode
- Draws gameplay entirely with primitive helpers and particle effects

If gameplay code grows further, prefer extracting helper functions into
`galaxian.c` or additional focused modules instead of making this file a second
engine layer.

### `src/state_gameover.c`

Game-over presentation and restart flow.

- Reads session values through the `gx_*` helpers
- Returns to gameplay on restart without resetting the whole app

### `src/particles.c` / `src/particles.h`

Small particle system used by gameplay effects.

- `particles_clear()`
- `particles_spawn()`
- `particles_update()`
- `particles_draw()`

Keep transient hit, explosion, and trail effects here rather than scattering
per-effect particle logic throughout multiple states.

### `src/drawing.c` / `src/drawing.h`

Renderer helper layer built on `get_renderer()`.

Current helpers include:

- Texture and text helpers:
  `draw_texture()`, `draw_texture_region()`, `draw_texture_ex()`, `draw_text()`
- Primitive helpers:
  `draw_rect()`, `draw_rect_outline()`, `draw_line()`, `draw_circle()`,
  `draw_circle_outline()`, `draw_triangle()`

If a gameplay feature needs a new reusable primitive, add it here instead of
calling raw SDL rendering APIs from state files.

### `src/resources.c` / `src/resources.h`

Centralized asset cache keyed by path.

- `res_load_texture()`
- `res_load_font()`
- `res_load_sound()`
- `res_load_music()`
- `res_free_all()`

Do not manually free assets returned by `res_load_*()`.

### `src/audio.c` / `src/audio.h`

SDL3_mixer wrapper layer.

- `audio_init()` / `audio_shutdown()`
- `play_sound()`
- `play_music()`, `play_music_once()`, `pause_music()`, `resume_music()`,
  `stop_music()`
- `set_master_volume()`, `set_music_volume()`
- `get_mixer()`

The current CMake setup enables WAV, MP3, and OGG Vorbis support.

## Current Gameplay Conventions

- Use the constants in `galaxian.h` for playfield dimensions, speeds, timings,
  colors, and font sizes
- The stage layout is a 46-enemy Galaxian-style formation, not a full 10x6
  filled grid
- The player is restricted to one on-screen bullet at a time
- Primitive rendering is the default for gameplay visuals; font rendering is
  the only regular text-based asset use in the current game
- Session-wide score data should go through the `gx_*` accessors, not ad hoc
  globals in state files

## Agent Notes

- Prefer keeping gameplay-specific logic out of `src/main.c`
- Keep reusable rendering behavior in `drawing.c` and reusable gameplay rules in
  `galaxian.c`
- Keep asset loading routed through `resources.c`
- Preserve the existing wrapper style instead of calling SDL subsystems
  directly from unrelated modules unless there is a clear need
- When changing gameplay rules, also review `SPEC.md` so the implementation and
  design doc stay aligned

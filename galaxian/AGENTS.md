# AGENTS.md

This file provides guidance to coding agents working in this repository.

## Build Commands

```powershell
# Configure (Visual Studio 18 2026 generator, x64)
cmake -S . -B build-codex -G "Visual Studio 18 2026" -A x64 -Wno-dev

# Build
cmake --build build-codex --config Debug

# Run
.\build-codex\Debug\hello_sdl.exe
```

Dependencies (`SDL3`, `SDL3_image`, `SDL3_mixer`, `SDL3_ttf`) live in `vendor/`
and are built via `add_subdirectory()`.

When adding new `.c` files, add them to the `add_executable()` call in
`CMakeLists.txt`.

## Project Architecture

SDL3 game template in C17 with a small engine-style module layout.

### `src/platform.c` / `src/platform.h`

Abstraction layer over SDL3 for windowing, timing, renderer access, and input.

- Window lifecycle: `init_window()`, `close_window()`, `is_window_ready()`,
  `window_should_close()`
- Frame timing: `get_deltatime()`, `get_fps()`
- Renderer access: `get_renderer()`
- Input polling with edge detection:
  `is_key_pressed()` vs `is_key_down()`,
  `is_mouse_button_pressed()` vs `is_mouse_button_down()`
- Per-frame engine hooks: `engine_begin_frame()`, `engine_process_event()`
- Shared utility types and enums: `vector2`, `rectangle`, keyboard/mouse/gamepad
  constants

All SDL-facing runtime state is kept internally in the platform layer.

### `src/game_state.c` / `src/game_state.h`

Simple state/scene system for gameplay flow.

- Register states with `game_state_register(STATE_ID, callbacks)`
- Each state can define `init`, `update(dt)`, `draw`, and `cleanup` callbacks
- Switch states with `game_state_switch()`
- Main loop forwards work through `game_state_update(dt)` and
  `game_state_draw()`
- Default enum entries include `STATE_MENU`, `STATE_GAMEPLAY`, and
  `STATE_PAUSE`

### `src/resources.c` / `src/resources.h`

Centralized asset cache keyed by path.

- `res_load_texture()`
- `res_load_font()`
- `res_load_sound()`
- `res_load_music()`
- `res_free_all()` releases every cached asset during shutdown

Do not manually free assets returned by `res_load_*()`.

### `src/drawing.c` / `src/drawing.h`

Rendering helpers built on `get_renderer()`.

- `draw_texture()`, `draw_texture_region()`, `draw_texture_ex()`
- `draw_text()`
- `draw_rect()`, `draw_rect_outline()`, `draw_line()`, `draw_circle()`

### `src/audio.c` / `src/audio.h`

Small wrapper layer over SDL3_mixer.

- `audio_init()` / `audio_shutdown()`
- `play_sound()`
- `play_music()`, `play_music_once()`, `pause_music()`, `resume_music()`,
  `stop_music()`
- `set_master_volume()`, `set_music_volume()`
- `get_mixer()` for direct low-level access when needed

The current CMake setup enables WAV, MP3, and OGG Vorbis support.

### `src/main.c`

Entry point that wires the template systems together. It currently registers a
single example gameplay state inline.

Main loop order:

1. `engine_begin_frame()`
2. `SDL_PollEvent()` + `engine_process_event()`
3. `get_deltatime()` -> `game_state_update(dt)`
4. Clear -> `game_state_draw()` -> `SDL_RenderPresent()`

Startup/shutdown order:

1. `init_window()`
2. `audio_init()`
3. register states
4. main loop
5. `game_state_shutdown()`
6. `res_free_all()`
7. `audio_shutdown()`
8. `close_window()`

## Agent Notes

- Prefer keeping game-specific logic in separate state files instead of growing
  `src/main.c`.
- Keep asset loading routed through `resources.c`.
- Preserve the existing wrapper style instead of calling SDL subsystems
  directly from unrelated modules unless there is a clear need.
